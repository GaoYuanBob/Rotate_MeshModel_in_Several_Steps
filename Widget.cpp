#include "Widget.h"
#include <meshlab/glarea.h>

Widget::Widget(QWidget *parent, EditTransformPlugin *_edit)
{
	ui.setupUi(this);
	this->edit = _edit;
}


Widget::~Widget()
{

}
