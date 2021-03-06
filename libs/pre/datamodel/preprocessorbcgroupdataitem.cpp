#include "preprocessorbcdataitem.h"
#include "preprocessorbcgroupdataitem.h"
#include "preprocessorgriddataitem.h"
#include "preprocessorgridtypedataitem.h"

#include <guibase/objectbrowserview.h>
#include <guibase/widget/itemmultiselectingdialog.h>
#include <guicore/pre/grid/grid.h>
#include <guicore/project/colorsource.h>
#include <guicore/project/projectdata.h>
#include <guicore/solverdef/solverdefinitionboundarycondition.h>
#include <misc/stringtool.h>
#include <misc/versionnumber.h>

#include <QDomNode>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItem>
#include <QXmlStreamWriter>

#include <iriclib.h>

PreProcessorBCGroupDataItem::PreProcessorBCGroupDataItem(PreProcessorDataItem* parent) :
	PreProcessorDataItem {tr("Boundary Condition"), QIcon(":/libs/guibase/images/iconFolder.png"), parent},
	m_deleteSelectedAction {new QAction(QIcon(":/libs/guibase/images/iconDeleteItem.png"), PreProcessorBCGroupDataItem::tr("Delete &Selected..."), this)},
	m_deleteAllAction {new QAction(QIcon(":/libs/guibase/images/iconDeleteItem.png"), PreProcessorBCGroupDataItem::tr("Delete &All..."), this)},
	m_dummyEditAction {new QAction(PreProcessorBCGroupDataItem::tr("&Edit Condition..."), this)},
	m_dummyDeleteAction {new QAction(PreProcessorBCGroupDataItem::tr("&Delete..."), this)},
	m_dummyAssignAction {new QAction(PreProcessorBCGroupDataItem::tr("&Assign Condition"), this)},
	m_dummyReleaseAction {new QAction(PreProcessorBCGroupDataItem::tr("&Release Condition"), this)},
	m_bcMenu {new QMenu(PreProcessorBCGroupDataItem::tr("&Boundary Condition"), mainWindow())}
{
	setupStandardItem(Checked, NotReorderable, NotDeletable);
	setSubPath("bc");

	m_colorSource = new ColorSource(this);

	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent->parent());
	SolverDefinitionGridType* gtype = gtItem->gridType();
	for (int i = 0; i < gtype->boundaryConditions().count(); ++i) {
		SolverDefinitionBoundaryCondition* bc = gtype->boundaryConditions().at(i);
		QString str(PreProcessorBCGroupDataItem::tr("Add %1"));
		QAction* addAction = new QAction(str.arg(bc->caption()), this);
		connect(addAction, SIGNAL(triggered()), this, SLOT(addCondition()));
		m_addActions.append(addAction);
	}
	connect(m_deleteSelectedAction, SIGNAL(triggered()), this, SLOT(deleteSelected()));
	connect(m_deleteAllAction, SIGNAL(triggered()), this, SLOT(deleteAll()));

	m_dummyEditAction->setDisabled(true);
	m_dummyDeleteAction->setDisabled(true);
	m_dummyAssignAction->setDisabled(true);
	m_dummyReleaseAction->setDisabled(true);
}

PreProcessorBCGroupDataItem::~PreProcessorBCGroupDataItem()
{
	delete m_colorSource;
}

void PreProcessorBCGroupDataItem::loadFromCgnsFile(const int fn)
{
	cg_iRIC_Init_BC_Names();
	Grid* grid = dynamic_cast<PreProcessorGridDataItem*>(parent())->grid();

	if (m_projectBuildNumber > 3507) {
		for (auto it = m_childItems.begin(); it != m_childItems.end(); ++it) {
			PreProcessorBCDataItem* bcItem = dynamic_cast<PreProcessorBCDataItem*>(*it);
			bcItem->loadFromCgnsFile(fn);
		}
	} else {
		// for backward compatibility.
		PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
		const QList<SolverDefinitionBoundaryCondition*>& conditions = gtItem->gridType()->boundaryConditions();
		for (int i = 0; i < conditions.count(); ++i) {
			SolverDefinitionBoundaryCondition* bc = conditions.at(i);
			int number;
			cg_iRIC_Read_BC_Count(const_cast<char*>(bc->name().c_str()), &number);
			for (int j = 0; j < number; ++j) {
				PreProcessorBCDataItem* bcItem = new PreProcessorBCDataItem(projectData()->solverDefinition(), bc, this);
				bcItem->setProjectNumber(j + 1);
				bcItem->setCgnsNumber(j + 1);
				bcItem->loadFromCgnsFile(fn);
				m_childItems.push_back(bcItem);
			}
		}
		assignActorZValues(m_zDepthRange);
		updateItemMap();
		emit itemsUpdated();
		emit itemsLoaded();
	}
}

void PreProcessorBCGroupDataItem::renumberItemsForProject()
{
	// set numbers.
	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
	const QList<SolverDefinitionBoundaryCondition*>& conditions = gtItem->gridType()->boundaryConditions();

	int number = 1;
	int condIndex = -1;
	auto it = m_childItems.begin();
	while (it != m_childItems.end()) {
		PreProcessorBCDataItem* tmpItem = dynamic_cast<PreProcessorBCDataItem*>(*it);
		SolverDefinitionBoundaryCondition* tmpbc = tmpItem->condition();
		int tmpindex = conditions.indexOf(tmpbc);
		if (tmpindex != condIndex) {
			number = 1;
			condIndex = tmpindex;
		} else {
			++ number;
		}
		tmpItem->setProjectNumber(number);
		++ it;
	}
}

const QList<QAction*> PreProcessorBCGroupDataItem::addActions() const
{
	return m_addActions;
}

void PreProcessorBCGroupDataItem::renumberItemsForCgns()
{
	// set numbers again.
	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
	const QList<SolverDefinitionBoundaryCondition*>& conditions = gtItem->gridType()->boundaryConditions();

	int number = 1;
	int condIndex = -1;
	auto it = m_childItems.begin();
	while (it != m_childItems.end()) {
		PreProcessorBCDataItem* tmpItem = dynamic_cast<PreProcessorBCDataItem*>(*it);
		if (tmpItem->isValid()) {
			SolverDefinitionBoundaryCondition* tmpbc = tmpItem->condition();
			int tmpindex = conditions.indexOf(tmpbc);
			if (tmpindex != condIndex) {
				number = 1;
				condIndex = tmpindex;
			} else {
				++ number;
			}
			tmpItem->setCgnsNumber(number);
		} else {
			tmpItem->setCgnsNumber(0);
		}
		++ it;
	}
}

void PreProcessorBCGroupDataItem::saveToCgnsFile(const int fn)
{
	renumberItemsForCgns();
	cg_iRIC_Clear_BC();
	for (int i = 0; i < m_childItems.size(); ++i) {
		m_childItems[i]->saveToCgnsFile(fn);
	}
}

void PreProcessorBCGroupDataItem::informGridUpdate()
{

}

void PreProcessorBCGroupDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	m_projectBuildNumber = projectData()->version().build();
	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
	QDomNodeList childNodes = node.childNodes();
	for (int i = 0; i < childNodes.count(); ++i) {
		QDomElement childElem = childNodes.at(i).toElement();
		std::string condType = iRIC::toStr(childElem.attribute("type"));
		SolverDefinitionBoundaryCondition* bc = gtItem->gridType()->boundaryCondition(condType);
		bool settingHidden = (childElem.attribute("settingHidden") == "true");
		PreProcessorBCDataItem* bcItem = new PreProcessorBCDataItem(projectData()->solverDefinition(), bc, this, settingHidden);
		bcItem->loadFromProjectMainFile(childElem);
		m_childItems.push_back(bcItem);
	}
	assignActorZValues(m_zDepthRange);
	updateItemMap();

	emit itemsUpdated();
	emit itemsLoaded();
}

void PreProcessorBCGroupDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	renumberItemsForProject();
	for (auto it = m_childItems.begin(); it != m_childItems.end(); ++it) {
		GraphicsWindowDataItem* item = *it;
		writer.writeStartElement("BoundaryCondition");
		item->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
}

void PreProcessorBCGroupDataItem::addCustomMenuItems(QMenu* menu)
{
	for (int i = 0; i < m_addActions.count(); ++i) {
		menu->addAction(m_addActions[i]);
	}
	menu->addSeparator();
	menu->addAction(m_deleteSelectedAction);
	menu->addAction(m_deleteAllAction);
}

void PreProcessorBCGroupDataItem::addCondition()
{
	QAction* a = dynamic_cast<QAction*>(sender());
	int index = 0;
	for (index = 0; index < m_addActions.count(); ++index) {
		if (m_addActions[index] == a) {break;}
	}

	PreProcessorBCDataItem* item = addCondition(index, true);
	if (item == 0) {return;}

	dataModel()->objectBrowserView()->expand(m_standardItem->index());
	dataModel()->objectBrowserView()->select(item->standardItem()->index());
	dataModel()->handleObjectBrowserSelection(item->standardItem()->index());
	bool ok = item->showDialog();
	if (! ok) {
		// Cancel button is clicked.
		delete item;
		dataModel()->handleObjectBrowserSelection(m_standardItem->index());
		updateItemMap();
	}
}

void PreProcessorBCGroupDataItem::deleteSelected()
{
	if (m_childItems.size() == 0) {
		QMessageBox::information(mainWindow(), tr("Information"), tr("There is no boundary condition."), QMessageBox::Ok);
		return;
	}

	auto items = m_childItems;
	std::vector<QString> names;
	for (GraphicsWindowDataItem* item : items) {
		names.push_back(item->standardItem()->text());
	}

	ItemMultiSelectingDialog dialog(mainWindow());
	dialog.setWindowTitle(tr("Delete selected boundary conditions"));
	dialog.setItems(names);
	int ret = dialog.exec();
	if (ret == QDialog::Rejected) {return;}

	auto settings = dialog.selectSettings();
	for (int i = 0; i < settings.size(); ++i) {
		if (settings.at(i)) {
			// delete the item
			delete items.at(i);
		}
	}
}

void PreProcessorBCGroupDataItem::deleteAll()
{
	int ret = QMessageBox::warning(mainWindow(), tr("Warning"), tr("Are you sure you want to delete all boundary conditions?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (ret == QMessageBox::No) {return;}

	auto items = m_childItems;
	for (auto item : items) {
		delete item;
	}
}

PreProcessorBCDataItem* PreProcessorBCGroupDataItem::addCondition(int index, bool hideSetting)
{
	if (index > m_addActions.count()) {return 0;}

	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent()->parent());
	SolverDefinitionGridType* gtype = gtItem->gridType();
	SolverDefinitionBoundaryCondition* bc = gtype->boundaryConditions().at(index);
	PreProcessorBCDataItem* item = new PreProcessorBCDataItem(projectData()->solverDefinition(), bc, this, hideSetting);
	QString tmpName(QString("New %1").arg(bc->englishCaption().c_str()));
	item->setName(tmpName);
	int number = 1;
	auto it = m_childItems.begin();
	while (it != m_childItems.end()) {
		PreProcessorBCDataItem* tmpItem = dynamic_cast<PreProcessorBCDataItem*>(*it);
		SolverDefinitionBoundaryCondition* tmpbc = tmpItem->condition();
		int tmpindex = gtype->boundaryConditions().indexOf(tmpbc);
		if (tmpindex == index) {++ number;}
		if (tmpindex > index) {break;}
		++ it;
	}
	item->setProjectNumber(number);
	item->setCgnsNumber(number);
	item->setColor(m_colorSource->getColor(m_childItems.size()));
	// add it simply.
	m_childItems.insert(it, item);
	PreProcessorGridDataItem* gItem = dynamic_cast<PreProcessorGridDataItem*>(parent());
	if (gItem->grid() != nullptr) {
		gItem->grid()->setModified();
	}

	// setup object browser tree again.
	int rows = m_standardItem->rowCount();
	for (int row = rows - 1; row >= 0; --row) {
		m_standardItem->takeRow(row);
	}
	for (int i = 0; i < m_childItems.size(); ++i) {
		GraphicsWindowDataItem* item = m_childItems.at(i);
		m_standardItem->appendRow(item->standardItem());
	}
	updateItemMap();
	assignActorZValues(m_zDepthRange);
	emit itemsUpdated();

	return item;
}

void PreProcessorBCGroupDataItem::updateZDepthRangeItemCount()
{
	m_zDepthRange.setItemCount(5);
}

QMenu* PreProcessorBCGroupDataItem::bcMenu() const
{
	return m_bcMenu;
}

void PreProcessorBCGroupDataItem::updateBCMenu(PreProcessorBCDataItem* item)
{
	m_bcMenu->clear();
	for (int i = 0; i < m_addActions.count(); ++i) {
		m_bcMenu->addAction(m_addActions[i]);
	}
	m_bcMenu->addSeparator();
	if (item == 0) {
		// add dummy menu items.
		m_bcMenu->addAction(m_dummyEditAction);
		m_bcMenu->addAction(m_dummyDeleteAction);
		m_bcMenu->addSeparator();
		m_bcMenu->addAction(m_dummyAssignAction);
		m_bcMenu->addAction(m_dummyReleaseAction);
	} else {
		m_bcMenu->addAction(item->editAction());
		m_bcMenu->addAction(item->deleteAction());
		m_bcMenu->addSeparator();
		m_bcMenu->addAction(item->assignAction());
		m_bcMenu->addAction(item->releaseAction());
	}
}

void PreProcessorBCGroupDataItem::clear()
{
	std::vector<GraphicsWindowDataItem*> itemCopys = m_childItems;
	for (int i = 0; i < itemCopys.size(); ++i) {
		GraphicsWindowDataItem* item = itemCopys[i];
		delete item;
	}
	m_childItems.clear();
	updateItemMap();
}

void PreProcessorBCGroupDataItem::clearPoints()
{
	std::vector<GraphicsWindowDataItem*> itemCopys = m_childItems;
	for (int i = 0; i < itemCopys.size(); ++i) {
		GraphicsWindowDataItem* item = itemCopys[i];
		PreProcessorBCDataItem* bcItem = dynamic_cast<PreProcessorBCDataItem*>(item);
		bcItem->clearPoints();
	}
}
