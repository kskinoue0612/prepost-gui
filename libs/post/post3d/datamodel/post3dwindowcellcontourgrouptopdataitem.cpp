#include "../../../guibase/objectbrowserview.h"
#include "post3dwindowcellcontourgrouptopdataitem.h"

#include "post3dwindowcontourgroupdataitem.h"
#include "post3dwindowcontourgroupsettingdialog.h"
#include "post3dwindowgridtypedataitem.h"
#include "post3dwindowzonedataitem.h"

#include <guibase/graphicsmisc.h>
#include <guibase/vtkdatasetattributestool.h>
#include <guicore/postcontainer/postzonedatacontainer.h>
#include <misc/iricundostack.h>
#include <misc/stringtool.h>

#include <QAction>
#include <QDomNode>
#include <QDomElement>
#include <QMenu>
#include <QMessageBox>
#include <QXmlStreamWriter>


#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractCells.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>

Post3dWindowCellContourGroupTopDataItem::Post3dWindowCellContourGroupTopDataItem(Post3dWindowDataItem* p) :
	Post3dWindowDataItem {tr("Contours (cell center)"), QIcon(":/libs/guibase/images/iconFolder.png"), p},
	m_zScale {1}
{
	setupStandardItem(Checked, NotReorderable, NotDeletable);

	PostZoneDataContainer* cont = dynamic_cast<Post3dWindowZoneDataItem*>(parent())->dataContainer();
	for (std::string name : vtkDataSetAttributesTool::getArrayNamesWithOneComponent(cont->data()->GetPointData())) {
		m_colorBarTitleMap.insert(name, name.c_str());
	}
	m_actor = vtkActor::New();
	m_mapper = vtkDataSetMapper::New();

	m_actor->SetMapper(m_mapper);

	renderer()->AddActor(m_actor);

}

Post3dWindowCellContourGroupTopDataItem::~Post3dWindowCellContourGroupTopDataItem()
{
	renderer()->RemoveActor(m_actor);

	m_actor->Delete();
	m_mapper->Delete();
}

void Post3dWindowCellContourGroupTopDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	// multi-contours
	QDomNodeList children = node.childNodes();
	for (int i = 0; i < children.count(); ++i) {
		QDomElement childElem = children.at(i).toElement();
		if (childElem.nodeName() == "ScalarBarTitles") {
			QDomNodeList titles = children.at(i).childNodes();
			for (int i = 0; i < titles.count(); ++i) {
				std::string val = iRIC::toStr(titles.at(i).toElement().attribute("value"));
				QString title = titles.at(i).toElement().attribute("title");
				m_colorBarTitleMap[val] = title;
			}
		} else if (childElem.nodeName() == "ContourGroup") {
			auto item = new Post3dWindowContourGroupDataItem(this);
			item->updateZScale(m_zScale);
			item->loadFromProjectMainFile(children.at(i));
			m_childItems.push_back(item);
		}
	}
}

void Post3dWindowCellContourGroupTopDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	// scalar bar titles
	writer.writeStartElement("ScalarBarTitles");
	QMapIterator<std::string, QString> i(m_colorBarTitleMap);
	while (i.hasNext()) {
		i.next();
		if (i.key().size() > 0) {
			writer.writeStartElement("ScalarBarTitle");
			writer.writeAttribute("value", i.key().c_str());
			writer.writeAttribute("title", i.value());
			writer.writeEndElement();
		}
	}
	writer.writeEndElement();

	// contours
	for (auto item : m_childItems) {
		writer.writeStartElement("ContourGroup");
		item->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
}

void Post3dWindowCellContourGroupTopDataItem::update()
{
	Post3dWindowGridTypeDataItem* gtItem = dynamic_cast<Post3dWindowGridTypeDataItem*>(parent()->parent());
	Post3dWindowZoneDataItem* zItem = dynamic_cast<Post3dWindowZoneDataItem*>(parent());

	vtkPointSet* ps = zItem->dataContainer()->data();

	auto cFilter = vtkSmartPointer<vtkExtractCells>::New();
	cFilter->SetInputData(ps);

	auto ids = vtkSmartPointer<vtkIdList>::New();
	ids->InsertNextId(0);
	ids->InsertNextId(1);
	ids->InsertNextId(2);
	cFilter->SetCellList(ids);

	m_mapper->SetInputConnection(cFilter->GetOutputPort());

	LookupTableContainer* lookup = gtItem->cellLookupTable("test1");
	if (lookup != nullptr) {
		m_mapper->SetLookupTable(lookup->vtkObj());
		m_mapper->UseLookupTableScalarRangeOn();
		m_mapper->SetScalarModeToUseCellFieldData();
		m_mapper->SelectColorArray("test1");
	}

	/*
	for (auto item : m_childItems) {
		auto child = dynamic_cast<Post3dWindowCellContourGroupDataItem*>(item);
		child->update();
	}
	*/
}

void Post3dWindowCellContourGroupTopDataItem::innerUpdateZScale(double scale)
{
	m_zScale = scale;
}

QDialog* Post3dWindowCellContourGroupTopDataItem::addDialog(QWidget* p)
{
	PostZoneDataContainer* zoneData = dynamic_cast<Post3dWindowZoneDataItem*>(parent())->dataContainer();
	if (zoneData == nullptr || zoneData->data() == nullptr) {
		return nullptr;
	}

	if (childItems().size() >= 4) {
		QMessageBox::warning(postProcessorWindow(), tr("Warning"), tr("A maximum of four contours may be defined."));
		return nullptr;
	}

	Post3dWindowContourGroupSettingDialog* dialog = new Post3dWindowContourGroupSettingDialog(p);
	dialog->setEnabled(true);
	Post3dWindowGridTypeDataItem* gtItem = dynamic_cast<Post3dWindowGridTypeDataItem*>(parent()->parent());
	dialog->setGridTypeDataItem(gtItem);
	dialog->setZoneData(zoneData);

	ScalarSettingContainer scalarSetting;
	QMap<QString, Post3dWindowFaceDataItem::Setting> map;

	Post3dWindowZoneDataItem* zItem = dynamic_cast<Post3dWindowZoneDataItem*>(parent());
	PostZoneDataContainer* c = zItem->dataContainer();
	scalarSetting.target = c->data()->GetPointData()->GetArrayName(0);
	scalarSetting.numberOfDivisions = 10;

	switch (childItems().size() % 4) {
	case 0:
		scalarSetting.scalarBarSetting.positionX = 0.8;
		scalarSetting.scalarBarSetting.positionY = 0.1;
		break;
	case 1:
		scalarSetting.scalarBarSetting.positionX = 0.1;
		scalarSetting.scalarBarSetting.positionY = 0.1;
		break;
	case 2:
		scalarSetting.scalarBarSetting.positionX = 0.1;
		scalarSetting.scalarBarSetting.positionY = 0.6;
		break;
	case 3:
		scalarSetting.scalarBarSetting.positionX = 0.8;
		scalarSetting.scalarBarSetting.positionY = 0.6;
		break;
	}

	LookupTableContainer *lookupTable = gtItem->nodeLookupTable(iRIC::toStr(scalarSetting.target));
	vtkSmartPointer<vtkScalarBarWidget> scalarBarWidget = vtkSmartPointer<vtkScalarBarWidget>::New();
	iRIC::setupScalarBarProperty(scalarBarWidget->GetScalarBarActor());

	dialog->setColorBarTitleMap(m_colorBarTitleMap);
	dialog->setScalarSetting(scalarSetting);
	dialog->setLookupTable(*lookupTable);
	dialog->setFaceMap(map);

	return dialog;
}

void Post3dWindowCellContourGroupTopDataItem::addCustomMenuItems(QMenu* menu)
{
	menu->addAction(dataModel()->objectBrowserView()->addAction());
}

/*
class Post3dWindowCellContourGroupTopDataItem::CreateCommand : public QUndoCommand
{
public:
	CreateCommand(Post3dWindowCellContourGroupTopDataItem *item, QDialog* propDialog)
		: QUndoCommand(QObject::tr("Create Contour"))
		, m_topItem(item)
		, m_firstDialog(true)
		, m_propDialog(propDialog)
		, m_undoCommand(nullptr)
	{}
	~CreateCommand() {
		delete m_undoCommand;
		if (! m_firstDialog) {
			delete m_propDialog;
		}
	}
	void redo() {
		m_item = new Post3dWindowContourGroupDataItem(m_topItem);
		m_item->setIsCommandExecuting(true);
		m_item->updateZScale(m_topItem->m_zScale);
		m_topItem->m_childItems.push_back(m_item);
		delete m_undoCommand;
		m_undoCommand = new QUndoCommand();
		m_item->undoCommands(m_propDialog, m_undoCommand);
		m_undoCommand->redo();
		m_item->setIsCommandExecuting(false);
	}
	void undo() {
		// don't delete original propDialog
		if (! m_firstDialog) {
			delete m_propDialog;
		}
		m_firstDialog = false;
		m_propDialog = m_item->propertyDialog(nullptr);
		m_undoCommand->undo();
		delete m_item;
		m_item = nullptr;
	}
private:
	Post3dWindowCellContourGroupTopDataItem* m_topItem;
	Post3dWindowContourGroupDataItem* m_item;
	QDialog* m_propDialog;
	QUndoCommand* m_undoCommand;
	bool m_firstDialog;
};
*/

void Post3dWindowCellContourGroupTopDataItem::handleAddDialogAccepted(QDialog* propDialog)
{
	// iRICUndoStack::instance().push(new CreateCommand(this, propDialog));
	// iRICUndoStack::instance().clear();
}
