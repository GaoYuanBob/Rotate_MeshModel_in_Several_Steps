#pragma once

#include <QObject>
#include <common/interfaces.h>

class EditTransformFactory : public QObject, public MeshEditInterfaceFactory
{
	Q_OBJECT
	MESHLAB_PLUGIN_IID_EXPORTER(MESH_EDIT_INTERFACE_FACTORY_IID)
	Q_INTERFACES(MeshEditInterfaceFactory)

public:
	EditTransformFactory();
	virtual ~EditTransformFactory() { delete editSample; }

	QList<QAction *> actions() const override;						//gets a list of actions available from this plugin
	MeshEditInterface* getMeshEditInterface(QAction *) override;	//get the edit tool for the given action
	QString getEditToolDescription(QAction *) override;				//get the description for the given action
	
private:
	QList<QAction *> actionList;
	QAction *editSample;
};