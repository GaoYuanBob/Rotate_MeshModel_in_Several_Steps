#pragma once

#include <common/interfaces.h>
#include "Widget.h"

class EditTransformPlugin : public QObject, public MeshEditInterface
{
	Q_OBJECT
	Q_INTERFACES(MeshEditInterface)
		
public:
	EditTransformPlugin();
    virtual ~EditTransformPlugin() = default;

	static const QString Info();

    bool StartEdit(MeshModel & , GLArea * /*parent*/, MLSceneGLSharedDataContext* /*cont*/) override;
	void suggestedRenderingData(MeshModel &, MLRenderingData& dt) override;

/// no implemention
	void EndEdit(MeshModel &, GLArea * , MLSceneGLSharedDataContext* ) override {};
	void Decorate(MeshModel & , GLArea * , QPainter * ) override {};
	void mousePressEvent(QMouseEvent *, MeshModel &, GLArea * ) override {};
	void mouseMoveEvent(QMouseEvent *, MeshModel &, GLArea * ) override {};
	void mouseReleaseEvent(QMouseEvent *, MeshModel &, GLArea *) override {};
	void keyReleaseEvent(QKeyEvent *, MeshModel &, GLArea *) override {};
/// no implemention

public slots:
	void slot_OK_pressed();
	void slot_rotate();

private:
    QPoint cur;
	QFont qFont;
	GLArea * gla;
	Widget * widget;
	MeshModel *m;

	double x_ketu, y_ketu, z_ketu;
	double theta;
	vcg::Matrix44<float> my_matrix;
	uint my_steps, cur_step;
};
