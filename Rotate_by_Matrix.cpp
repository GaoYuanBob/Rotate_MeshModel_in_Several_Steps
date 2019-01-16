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
		printf("模型没有正确加载!!!\n");
		return;
	}
	
	// widget->ui.lineEdit_00->setReadOnly(true);
	widget->ui.lineEdit_11->setEnabled(false);
	widget->ui.lineEdit_21->setEnabled(false);

	widget->ui.lineEdit_steps->setEnabled(false);

	// 绕平行于 x 轴的旋转轴的旋转矩阵为 
	// float for_mat[16] = { 1,    0,     0,      0,
	// 						 0, cosine, -sine, (1 - cosine) * y_ketu + z_ketu * sine,
	// 						 0, sine,  cosine, (1 - cosine) * z_ketu - y_ketu * sine,
	// 						 0,    0,     0,      1 };

	// 只需要读 11, 21, 13, 14 即可
	const auto cosine = widget->ui.lineEdit_11->text().toDouble();	// 0.996626;
	const auto sine = widget->ui.lineEdit_21->text().toDouble();	// 0.0820732;
	// 角度变了之后，位移要重新算
	const auto trans_y = (1 - cosine) * y_ketu + z_ketu * sine;
	const auto trans_z = (1 - cosine) * z_ketu - y_ketu * sine;

	theta = fabs(acos(cosine));	// my_matrix[1][1] 角度一定是正的

	float for_mat[16] = { 1,    0,     0,      0,
						  0, cosine, -sine, trans_y,
						  0, sine,  cosine, trans_z,
						  0,    0,     0,      1 };
	my_matrix = Matrix44<float>(for_mat);
	printf("旋转矩阵为\n");
	my_matrix.print();

	my_steps = widget->ui.lineEdit_steps->text().toUInt();
	if(my_steps < 1)
	{
		QMessageBox::critical(widget, QString::fromLocal8Bit("严重错误提示框"), QString::fromLocal8Bit("发现一个严重错误！步骤不是正整数!"), QMessageBox::Close);
		return;
	}
	printf("要将旋转矩阵分为 %d 步执行\n", my_steps);

	QString progress_text = "0 / ";
	progress_text += QString::number(my_steps);
	widget->ui.progress->setText(progress_text);
}

void EditTransformPlugin::slot_rotate()
{
	if(my_steps == 1)
	{
		m->cm.Tr = my_matrix;	// 这样就是一步完成
		QMessageBox::information(widget, QString::fromLocal8Bit("完成提示框"), QString::fromLocal8Bit("旋转已完成!"), QMessageBox::Close);
		return;
	}
	if(cur_step == my_steps + 1)
	{
		QMessageBox::information(widget, QString::fromLocal8Bit("完成提示框"), QString::fromLocal8Bit("旋转已完成!"), QMessageBox::Close);
		return;
	}
	// 算出第 cur_step 步对应的旋转矩阵（相对于最初始情况）
	// 需要利用 cosine 算出角度 theta, 然后 theta / 10 * cur_step 算出第 cur_step 步的旋转矩阵（相对于最初始情况）

	const auto cur_angle = theta * cur_step / static_cast<double>(my_steps);
	printf("第 %d 步对应的旋转角度为 %.2lf\n", cur_step, cur_angle);
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
	// printf("第 %d 步对应的旋转矩阵为\n", cur_step);
	// cur_matrix.print();

	m->cm.Tr = cur_matrix;

	auto progress_text = QString::number(cur_step);
	progress_text += " / ";
	progress_text += QString::number(my_steps);
	widget->ui.progress->setText(progress_text);

	++cur_step;

	gla->update();	// !!!!!! 加上这个就是点一下就刷新一下（也就是模型的变换会刷新，点一下转一下） !!!!!!
}

/// 输入16个浮点数作为旋转矩阵，对当前MeshModel进行旋转(平移值原点、旋转、平移回去)（变换矩阵是从MeshLab中得到的） GY - 2019.1.14
// 其实只需要cosine，或者sine值（也就是旋转角度），和旋转轴
bool EditTransformPlugin::StartEdit(MeshModel & _m, GLArea * _gla, MLSceneGLSharedDataContext* _ctx)
{
	gla = _gla;
	m = &_m;

	cur_step = 1;
	x_ketu = 53.0, y_ketu = -65.0, z_ketu = -6.0;	// 需要这个来计算每一步的位移

	if(!widget)
	{
		widget = new Widget(gla->window(), this);
		connect(widget->ui.OK_Button, SIGNAL(clicked()), this, SLOT(slot_OK_pressed()));
		connect(widget->ui.Rotate_Button, SIGNAL(clicked()), this, SLOT(slot_rotate()));

		widget->ui.lineEdit_00->setEnabled(false); // 让这个空不能填写东西
		widget->ui.lineEdit_01->setEnabled(false);
		widget->ui.lineEdit_01->setReadOnly(true);
		widget->ui.lineEdit_02->setEnabled(false);
		widget->ui.lineEdit_03->setEnabled(false);
		widget->ui.lineEdit_10->setEnabled(false);
		// widget->ui.lineEdit_11->setEnabled(false);	// cosine
		widget->ui.lineEdit_12->setEnabled(false);
		widget->ui.lineEdit_13->setEnabled(false);		// 混合后 y 的平移
		widget->ui.lineEdit_20->setEnabled(false);
		// widget->ui.lineEdit_21->setEnabled(false);	// sine
		widget->ui.lineEdit_22->setEnabled(false);
		widget->ui.lineEdit_23->setEnabled(false);		// 混合后 z 的平移
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