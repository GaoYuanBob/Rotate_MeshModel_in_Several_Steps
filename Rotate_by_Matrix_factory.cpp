#include "Rotate_by_Matrix_factory.h"
#include "Rotate_by_Matrix.h"

EditTransformFactory::EditTransformFactory()
{
	editSample = new QAction(QIcon(":/images/icon_Rotate.png"),"Rotate current mesh by matrix in several steps!! - GY", this);
	actionList << editSample;
	foreach(QAction *editAction, actionList)
		editAction->setCheckable(true);
}
	
//gets a list of actions available from this plugin
QList<QAction *> EditTransformFactory::actions() const
{
	return actionList;
}

//get the edit tool for the given action
MeshEditInterface* EditTransformFactory::getMeshEditInterface(QAction *action)
{
	if(action == editSample)
		return new EditTransformPlugin();
	assert(0); //should never be asked for an action that isnt here
	return nullptr;
}

QString EditTransformFactory::getEditToolDescription(QAction *)
{
	return EditTransformPlugin::Info();
}

MESHLAB_PLUGIN_NAME_EXPORTER(EditTransformFactory)
