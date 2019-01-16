#pragma once
#include "qwidget.h"
#include "ui_Widget.h"

class EditTransformPlugin;
class Widget :	public QWidget
{
public:
	Widget(QWidget *parent, EditTransformPlugin *_edit);
	~Widget();
	
	Ui::Form ui;

private:
	EditTransformPlugin *edit;
};

