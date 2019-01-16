#include <iostream>
#include <meshlab/glarea.h>
#include "Rotate_by_Matrix.h"
#include <wrap/qt/gl_label.h>
#include <cstdio>
#include <cstring>

using namespace vcg;

EditTransformPlugin::EditTransformPlugin(): gla(nullptr), widget(nullptr), my_steps(0), cur_step(1), theta(0)
{
	qFont.setFamily("Helvetica");
	qFont.setPixelSize(12);
}

const QString EditTransformPlugin::Info()
{
	return tr("Laplace Deformation on a model!");
}

void EditTransformPlugin::slot_OK_pressed()
{
	if(!m)
	{
		printf("ģ��û����ȷ����!!!\n");
		return;
	}
	
	// widget->ui.lineEdit_00->setReadOnly(true);
	widget->ui.lineEdit_11->setEnabled(false);
	widget->ui.lineEdit_21->setEnabled(false);

	widget->ui.lineEdit_steps->setEnabled(false);

	// ��ƽ���� x �����ת�����ת����Ϊ 
	// float for_mat[16] = { 1,    0,     0,      0,
	// 						 0, cosine, -sine, (1 - cosine) * y_ketu + z_ketu * sine,
	// 						 0, sine,  cosine, (1 - cosine) * z_ketu - y_ketu * sine,
	// 						 0,    0,     0,      1 };

	// ֻ��Ҫ�� 11, 21, 13, 14 ����
	const auto cosine = widget->ui.lineEdit_11->text().toDouble();	// 0.996626;
	const auto sine = widget->ui.lineEdit_21->text().toDouble();	// 0.0820732;
	// �Ƕȱ���֮��λ��Ҫ������
	const auto trans_y = (1 - cosine) * y_ketu + z_ketu * sine;
	const auto trans_z = (1 - cosine) * z_ketu - y_ketu * sine;

	theta = fabs(acos(cosine));	// my_matrix[1][1] �Ƕ�һ��������

	float for_mat[16] = { 1,    0,     0,      0,
						  0, cosine, -sine, trans_y,
						  0, sine,  cosine, trans_z,
						  0,    0,     0,      1 };
	my_matrix = Matrix44<float>(for_mat);
	printf("��ת����Ϊ\n");
	my_matrix.print();

	my_steps = widget->ui.lineEdit_steps->text().toUInt();
	if(my_steps < 1)
	{
		QMessageBox::critical(widget, QString::fromLocal8Bit("���ش�����ʾ��"), QString::fromLocal8Bit("����һ�����ش��󣡲��費��������!"), QMessageBox::Close);
		return;
	}
	printf("Ҫ����ת�����Ϊ %d ��ִ��\n", my_steps);

	QString progress_text = "0 / ";
	progress_text += QString::number(my_steps);
	widget->ui.progress->setText(progress_text);
}

void EditTransformPlugin::slot_rotate()
{
	if(my_steps == 1)
	{
		m->cm.Tr = my_matrix;	// ��������һ�����
		QMessageBox::information(widget, QString::fromLocal8Bit("�����ʾ��"), QString::fromLocal8Bit("��ת�����!"), QMessageBox::Close);
		return;
	}
	if(cur_step == my_steps + 1)
	{
		QMessageBox::information(widget, QString::fromLocal8Bit("�����ʾ��"), QString::fromLocal8Bit("��ת�����!"), QMessageBox::Close);
		return;
	}
	// ����� cur_step ����Ӧ����ת������������ʼ�����
	// ��Ҫ���� cosine ����Ƕ� theta, Ȼ�� theta / 10 * cur_step ����� cur_step ������ת������������ʼ�����

	const auto cur_angle = theta * cur_step / static_cast<double>(my_steps);
	printf("�� %d ����Ӧ����ת�Ƕ�Ϊ %.2lf\n", cur_step, cur_angle);
	const auto cosine_theta = cos(cur_angle);
	const auto sine_theta = sin(cur_angle);

	const auto trans_y = (1 - cosine_theta) * y_ketu + z_ketu * sine_theta;
	const auto trans_z = (1 - cosine_theta) * z_ketu - y_ketu * sine_theta;

	std::cout << "cosine_theta = " << cosine_theta << ", sine_theta = " << sine_theta << std::endl;

	float for_mat[16] = { 1,    0,     0,      0,
						  0, cosine_theta, -sine_theta, trans_y /*my_matrix[1][3]*/,
						  0, sine_theta,  cosine_theta, trans_z /*my_matrix[2][3]*/,
						  0,    0,     0,      1 };
	auto cur_matrix = Matrix44<float>(for_mat);
	// printf("�� %d ����Ӧ����ת����Ϊ\n", cur_step);
	// cur_matrix.print();

	m->cm.Tr = cur_matrix;

	auto progress_text = QString::number(cur_step);
	progress_text += " / ";
	progress_text += QString::number(my_steps);
	widget->ui.progress->setText(progress_text);

	++cur_step;

	gla->update();	// !!!!!! ����������ǵ�һ�¾�ˢ��һ�£�Ҳ����ģ�͵ı任��ˢ�£���һ��תһ�£� !!!!!!
}

/// ����16����������Ϊ��ת���󣬶Ե�ǰMeshModel������ת(ƽ��ֵԭ�㡢��ת��ƽ�ƻ�ȥ)���任�����Ǵ�MeshLab�еõ��ģ� GY - 2019.1.14
// ��ʵֻ��Ҫcosine������sineֵ��Ҳ������ת�Ƕȣ�������ת��
bool EditTransformPlugin::StartEdit(MeshModel & _m, GLArea * _gla, MLSceneGLSharedDataContext* _ctx)
{
	gla = _gla;
	m = &_m;

	cur_step = 1;
	x_ketu = 53.0, y_ketu = -65.0, z_ketu = -6.0;	// ��Ҫ���������ÿһ����λ��

	if(!widget)
	{
		widget = new Widget(gla->window(), this);
		connect(widget->ui.OK_Button, SIGNAL(clicked()), this, SLOT(slot_OK_pressed()));
		connect(widget->ui.Rotate_Button, SIGNAL(clicked()), this, SLOT(slot_rotate()));

		widget->ui.lineEdit_00->setEnabled(false); // ������ղ�����д����
		widget->ui.lineEdit_01->setEnabled(false);
		widget->ui.lineEdit_01->setReadOnly(true);
		widget->ui.lineEdit_02->setEnabled(false);
		widget->ui.lineEdit_03->setEnabled(false);
		widget->ui.lineEdit_10->setEnabled(false);
		// widget->ui.lineEdit_11->setEnabled(false);	// cosine
		widget->ui.lineEdit_12->setEnabled(false);
		widget->ui.lineEdit_13->setEnabled(false);		// ��Ϻ� y ��ƽ��
		widget->ui.lineEdit_20->setEnabled(false);
		// widget->ui.lineEdit_21->setEnabled(false);	// sine
		widget->ui.lineEdit_22->setEnabled(false);
		widget->ui.lineEdit_23->setEnabled(false);		// ��Ϻ� z ��ƽ��
		widget->ui.lineEdit_30->setEnabled(false);
		widget->ui.lineEdit_31->setEnabled(false);
		widget->ui.lineEdit_32->setEnabled(false);
		widget->ui.lineEdit_33->setEnabled(false);
	}
	
	widget->show();

	return true;
}

void EditTransformPlugin::suggestedRenderingData(MeshModel &m, MLRenderingData& dt)
{
	if (m.cm.VN() == 0)
		return;
	auto pr = MLRenderingData::PR_SOLID;	// MLRenderingData::PRIMITIVE_MODALITY
	if (m.cm.FN() > 0)
		pr = MLRenderingData::PR_SOLID;

	MLRenderingData::RendAtts atts;
	atts[MLRenderingData::ATT_NAMES::ATT_VERTTEXTURE] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_VERTPOSITION] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_VERTCOLOR] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_VERTNORMAL] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_FACENORMAL] = true;
	atts[MLRenderingData::ATT_NAMES::ATT_FACECOLOR] = true;

	MLPerViewGLOptions opts;
	dt.get(opts);
	opts._sel_enabled = true;
	opts._face_sel = true;
	opts._vertex_sel = true;
	dt.set(opts);

	dt.set(pr, atts);
}