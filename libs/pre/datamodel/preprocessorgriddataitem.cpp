#include <guicore/project/projectdata.h>
#include <guicore/project/projectcgnsfile.h>
#include "preprocessorgriddataitem.h"
#include "preprocessorgridshapedataitem.h"
#include "preprocessorgridtypedataitem.h"
#include "preprocessorgridandgridcreatingconditiondataitem.h"
#include "preprocessorgridrelatedconditionnodegroupdataitem.h"
#include "preprocessorgridrelatedconditioncellgroupdataitem.h"
#include "preprocessorbcgroupdataitem.h"
#include "preprocessorgridrelatedconditionnodedataitem.h"
#include "preprocessorgridattributemappingsettingtopdataitem.h"
#include "preprocessorrawdatatopdataitem.h"
#include <guicore/solverdef/solverdefinitiongridtype.h>
#include <guicore/solverdef/solverdefinitiongridrelatedcondition.h>
#include <misc/mathsupport.h>
#include <guicore/pre/grid/grid2d.h>
#include <guicore/pre/grid/structured2dgrid.h>
#include <guicore/pre/grid/unstructured2dgrid.h>
#include "../factory/gridimporterfactory.h"
#include "../factory/gridexporterfactory.h"
#include <guicore/pre/grid/gridimporterinterface.h>
#include <guicore/pre/grid/gridexporterinterface.h>
#include "../gridimporter/cgnsgridimporter.h"
#include "../gridexporter/cgnsgridexporter.h"
#include "../preprocessordatamodel.h"
#include <guicore/misc/mouseboundingbox.h>
#include <guicore/base/iricmainwindowinterface.h>
#include <misc/iricundostack.h>
#include "../preprocessorgraphicsview.h"
#include "../preprocessorwindow.h"
#include <guicore/pre/gridcond/base/gridrelatedconditioncontainer.h>
#include "../subwindow/gridbirdeyewindow/gridbirdeyewindow.h"
#include <guicore/pre/base/preprocessorgraphicsviewinterface.h>

#include <misc/xmlsupport.h>
#include <misc/errormessage.h>
#include <misc/lastiodirectory.h>

#include <vtkVertex.h>
#include <vtkTriangle.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>
#include <vtkLine.h>

#include <QIcon>
#include <QStandardItem>
#include <QDomNode>
#include <QSettings>
#include <QXmlStreamWriter>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QSignalMapper>
#include <QGraphicsItem>
#include <QMouseEvent>
#include <QMessageBox>
#include <QSet>
#include <QtGlobal>
#include <QStatusBar>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QTextStream>

#include <cgnslib.h>
#include <iriclib.h>

PreProcessorGridDataItem::PreProcessorGridDataItem(PreProcessorDataItem* parent)
	: PreProcessorGridDataItemInterface(parent)
{
	m_isDeletable = false;
	m_standardItem->setCheckable(true);
	m_standardItem->setCheckState(Qt::Checked);

	m_standardItemCopy = m_standardItem->clone();

	m_grid = 0;

	m_nodeDataItem = 0;
	m_cellDataItem = 0;
	m_shiftPressed = false;

	// Set cursors for mouse view change events.
	m_addPixmap = QPixmap(":/libs/guibase/images/cursorAdd.png");
	m_addCursor = QCursor(m_addPixmap, 0, 0);

	m_birdEyeWindow = 0;

	setupActors();
	setupActions();
}

PreProcessorGridDataItem::~PreProcessorGridDataItem()
{
	renderer()->RemoveActor(m_regionActor);
	renderer()->RemoveActor(m_selectedVerticesActor);
	renderer()->RemoveActor(m_selectedCellsActor);
	renderer()->RemoveActor(m_selectedCellsLinesActor);
	renderer()->RemoveActor(m_selectedEdgesActor);
	closeBirdEyeWindow();
	if (m_bcGroupDataItem != 0){
		delete m_bcGroupDataItem;
	}
	if (m_grid != 0){
		delete m_grid;
	}
	delete m_menu;
}

void PreProcessorGridDataItem::doLoadFromProjectMainFile(const QDomNode& node)
{
	QDomNode shapeNode = iRIC::getChildNode(node, "Shape");
	if (! shapeNode.isNull()){m_shapeDataItem->loadFromProjectMainFile(shapeNode);}

	QDomNode nodeNode = iRIC::getChildNode(node, "NodeAttributes");
	if (! nodeNode.isNull()){m_nodeGroupDataItem->loadFromProjectMainFile(nodeNode);}

	QDomNode cellNode = iRIC::getChildNode(node, "CellAttributes");
	if (! cellNode.isNull()){m_cellGroupDataItem->loadFromProjectMainFile(cellNode);}

	QDomNode bcNode = iRIC::getChildNode(node, "BoundaryConditions");
	if (! bcNode.isNull() && m_bcGroupDataItem != 0){
		m_bcGroupDataItem->loadFromProjectMainFile(bcNode);
	}
}

void PreProcessorGridDataItem::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
	writer.writeStartElement("Shape");
	m_shapeDataItem->saveToProjectMainFile(writer);
	writer.writeEndElement();

	writer.writeStartElement("NodeAttributes");
	m_nodeGroupDataItem->saveToProjectMainFile(writer);
	writer.writeEndElement();

	writer.writeStartElement("CellAttributes");
	m_cellGroupDataItem->saveToProjectMainFile(writer);
	writer.writeEndElement();

	if (m_bcGroupDataItem != 0){
		writer.writeStartElement("BoundaryConditions");
		m_bcGroupDataItem->saveToProjectMainFile(writer);
		writer.writeEndElement();
	}
}

void PreProcessorGridDataItem::loadFromCgnsFile(const int fn)
{
	QString zoneName = dynamic_cast<PreProcessorGridAndGridCreatingConditionDataItem*>(parent())->zoneName();
	// There is only one base_t node.
	int B;
	int nzones;
	cg_iRIC_GotoBase(fn, &B);
	cg_nzones(fn, B, &nzones);
	char zonename[ProjectCgnsFile::BUFFERLEN];
	cgsize_t size[9];
	for (int i = 1; i <= nzones; ++i){
		// read zone information.
		cg_zone_read(fn, 1, i, zonename, size);
		if (zoneName == zonename){
			// I've found the zone!
			// check the type of the grid contained in this zone.
			ZoneType_t type;
			cg_zone_type(fn, 1, i, &type);
			// the grid type knows what kind of grid it should have.
			SolverDefinitionGridType* gridType = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent())->gridType();
			// create grid first.
			m_grid = gridType->createEmptyGrid();
			m_grid->setParent(this);
			m_grid->setZoneName(zoneName);

			// Now, memory is allocated. load data.
			m_grid->loadFromCgnsFile(fn);
		}
	}
	for (auto it = m_childItems.begin(); it != m_childItems.end(); ++it){
		(*it)->loadFromCgnsFile(fn);
	}
	// loading data finished.
	// now call vtk related functions, and render new grid.
	updateSimplefiedGrid();
	finishGridLoading();
	updateActionStatus();
	updateObjectBrowserTree();
}

void PreProcessorGridDataItem::saveToCgnsFile(const int fn)
{
	if (m_grid != 0){
		bool modified = m_grid->isModified();
		m_grid->saveToCgnsFile(fn);
		if (m_bcGroupDataItem != 0 && modified){
			try {
				m_bcGroupDataItem->saveToCgnsFile(fn);
			} catch (ErrorMessage& m){
				m_grid->setModified();
				throw m;
			}
		}
	}
}

void PreProcessorGridDataItem::closeCgnsFile()
{
	if (m_grid != 0){
		delete m_grid;
		m_grid = 0;
		m_shapeDataItem->informGridUpdate();
		m_nodeGroupDataItem->informGridUpdate();
		m_cellGroupDataItem->informGridUpdate();
		m_bcGroupDataItem->informGridUpdate();
	}
	updateRegionPolyData();
	updateObjectBrowserTree();
	updateActionStatus();
}

void PreProcessorGridDataItem::addCustomMenuItems(QMenu* menu)
{
	menu->addAction(m_importAction);
	menu->addAction(m_exportAction);

	menu->addSeparator();
	menu->addAction(m_deleteAction);
}

void PreProcessorGridDataItem::importGrid()
{
	iRICMainWindowInterface* mw = dataModel()->iricMainWindow();
	if (mw->isSolverRunning()){
		mw->warnSolverRunning();
		return;
	}
	QString dir = LastIODirectory::get();
	QString selectedFilter;
	QStringList filters;
	QList<GridImporterInterface*> importers;

	Grid* tmpgrid = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent())->gridType()->emptyGrid();
	const QList<GridImporterInterface*> importerList = GridImporterFactory::instance().list(tmpgrid->gridType());
	for (auto it = importerList.begin(); it != importerList.end(); ++it){
		QStringList flist = (*it)->fileDialogFilters();
		for (auto fit = flist.begin(); fit != flist.end(); ++fit){
			filters.append(*fit);
			importers.append(*it);
		}
	}

	// Select the file to import.
	QString filename = QFileDialog::getOpenFileName(projectData()->mainWindow(), tr("Select file to import"), dir, filters.join(";;"), &selectedFilter);
	if (filename.isNull()){return;}
	GridImporterInterface* importer = 0;
	for (int i = 0; i < filters.count(); ++i){
		if (filters[i] == selectedFilter){
			importer = importers[i];
		}
	}
	Q_ASSERT(importer != 0);

	// execute import.
	if (m_grid){
		// delete the current grid first.
		silentDeleteGrid();
	}
	// create new empty grid.
	Grid* importedGrid = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent())->gridType()->createEmptyGrid();
	// set parent.
	importedGrid->setParent(this);
	// set zone name.
	QString zname = dynamic_cast<PreProcessorGridAndGridCreatingConditionDataItem*>(parent())->zoneName();
	importedGrid->setZoneName(zname);
	// now, import grid data.
	bool ret = true;
	CgnsGridImporter* cgnsImpoter = dynamic_cast<CgnsGridImporter*> (importer);
	if (cgnsImpoter != 0){
		// CGNS importer is a little special.
		// Boundary condition should be imported too.
		QString tmpname;
		int fn, B, zoneid;
		// create temporary CGNS file.
		bool internal_ret = cgnsImpoter->openCgnsFileForImporting(importedGrid, filename, tmpname, fn, B, zoneid, mainWindow());
		if (! internal_ret){goto IMPORT_ERROR_BEFORE_OPEN;}

		// load grid
		internal_ret = importedGrid->loadFromCgnsFile(fn, B, zoneid);
		if (! internal_ret){goto IMPORT_ERROR_AFTER_OPEN;}

		m_grid = importedGrid;

		// import boundary condition
		if (m_bcGroupDataItem != 0){
			m_bcGroupDataItem->clear();
			m_bcGroupDataItem->loadFromCgnsFile(fn);
		}
		cgnsImpoter->closeAndRemoveTempCgnsFile(fn, tmpname);
		goto IMPORT_SUCCEED;

IMPORT_ERROR_AFTER_OPEN:
		cgnsImpoter->closeAndRemoveTempCgnsFile(fn, tmpname);
IMPORT_ERROR_BEFORE_OPEN:
		ret = false;
IMPORT_SUCCEED:
		;

	} else {
		ret = importer->import(importedGrid, filename, selectedFilter, projectData()->mainWindow());
		m_grid = importedGrid;
	}

	if (! ret){
		// import failed.
		delete m_grid;
		m_grid = 0;
		QMessageBox::critical(dataModel()->mainWindow(), tr("Error"), tr("Importing grid failed."));
		return;
	}
	// import succeeded.
	m_grid->setModified();
	// loading data finished.
	// now call vtk related functions, and render new grid.
	finishGridLoading();

	QFileInfo finfo(filename);
	LastIODirectory::set(finfo.absolutePath());
	dataModel()->fit();

	m_shapeDataItem->updateActionStatus();
	updateObjectBrowserTree();

	updateActionStatus();

	mainWindow()->setFocus();

	informGridChange();
}

void PreProcessorGridDataItem::exportGrid()
{
	// Check whether the grid shape is valid.
	QSettings settings;
	settings.beginGroup("gridcheck");

	QFile logFile(projectData()->absoluteFileName("gridcheck.txt"));
	logFile.open(QFile::WriteOnly | QFile::Text);
	QTextStream logStream(&logFile);
	if (settings.value("beforeexport", true).value<bool>()){
		// execute checking before exporting.
		QStringList messages = m_grid->checkShape(logStream);
		if (messages.count() > 0) {
			QString msg = tr("The following problems found in this grid. Do you really want to export the grid?");
			msg += "<ul>";
			for (int i = 0; i < messages.count(); ++i){
				QString tmpmsg = messages.at(i);
				msg += "<li>" + tmpmsg + "</li>";
			}
			msg += "</ul>";
			QString logFileName = projectData()->absoluteFileName("gridcheck.txt");
			msg.append(QString("<a href=\"%1\">").arg(QString("file:///").append(logFileName))).append(tr("Show Detail")).append("</a>");
			int ret = QMessageBox::warning(mainWindow(), tr("Warning"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
			if (ret == QMessageBox::No) {
				return;
			}
		}
	}
	logFile.close();
	QString dir = LastIODirectory::get();
	QString selectedFilter;
	QStringList filters;
	QList<GridExporterInterface*> exporters;

	const QList<GridExporterInterface*> exporterList = GridExporterFactory::instance().list(m_grid->gridType());
	for (auto it = exporterList.begin(); it != exporterList.end(); ++it){
		QStringList flist = (*it)->fileDialogFilters();
		for (auto fit = flist.begin(); fit != flist.end(); ++fit){
			filters.append(*fit);
			exporters.append(*it);
		}
	}

	// Select the file to import.
	QString filename = QFileDialog::getSaveFileName(projectData()->mainWindow(), tr("Select File to Export"), dir, filters.join(";;"), &selectedFilter);
	if (filename.isNull()){return;}
	GridExporterInterface* exporter = 0;
	for (int i = 0; i < filters.count(); ++i){
		if (filters[i] == selectedFilter){
			exporter = exporters[i];
		}
	}
	Q_ASSERT(exporter != 0);

	// execute export.
	projectData()->mainWindow()->statusBar()->showMessage(tr("Exporting Grid..."));
	bool ret;
	CgnsGridExporter* cgnsExporter = dynamic_cast<CgnsGridExporter*>(exporter);
	if (cgnsExporter != 0){
		// CGNS exporter is a little special.
		// Boundary condition should be exported too.
		QString tmpname;
		int fn, B;
		// create temporary CGNS file.
		bool internal_ret = cgnsExporter->createTempCgns(m_grid, tmpname, fn, B);
		if (! internal_ret){goto EXPORT_ERROR_BEFORE_OPEN;}

		// write to the iRICZone.
		internal_ret = m_grid->saveToCgnsFile(fn, B, "iRICZone");
		if (! internal_ret){goto EXPORT_ERROR_AFTER_OPEN;}

		// output boundary condition
		if (m_bcGroupDataItem != 0){
			try {
				m_bcGroupDataItem->saveToCgnsFile(fn);
			} catch (ErrorMessage& m){
				QMessageBox::critical(mainWindow(), tr("Error"), m);
				goto EXPORT_ERROR_AFTER_OPEN;
			}
		}
		ret = cgnsExporter->closeAndMoveCgns(tmpname, fn, filename);
		goto EXPORT_SUCCEED;

EXPORT_ERROR_AFTER_OPEN:
		cg_close(fn);
		QFile::remove(tmpname);
EXPORT_ERROR_BEFORE_OPEN:
		ret = false;
EXPORT_SUCCEED:
		;
	} else {
		ret = exporter->doExport(m_grid, filename, selectedFilter, projectData()->mainWindow());
	}
	if (ret){
		// exporting succeeded.
		projectData()->mainWindow()->statusBar()->showMessage(tr("Grid successfully exported to %1.").arg(QDir::toNativeSeparators(filename)), iRICMainWindowInterface::STATUSBAR_DISPLAYTIME);
	}else{
		// exporting failed.
		projectData()->mainWindow()->statusBar()->clearMessage();
		QMessageBox::critical(mainWindow(), tr("Error"), tr("Exporting grid to %1 failed.").arg(QDir::toNativeSeparators(filename)));
	}
	QFileInfo finfo(filename);
	LastIODirectory::set(finfo.absolutePath());
}

void PreProcessorGridDataItem::showDisplaySettingDialog()
{
	m_shapeDataItem->showPropertyDialog();
}

void PreProcessorGridDataItem::deleteGrid()
{
	if (m_grid == 0){return;}
	iRICMainWindowInterface* mw = dataModel()->iricMainWindow();
	if (mw->isSolverRunning()){
		mw->warnSolverRunning();
		return;
	}
	QMessageBox::StandardButton button = QMessageBox::warning(preProcessorWindow(), tr("Warning"), tr("Are you sure you want to discard the grid?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (button == QMessageBox::No){return;}
	silentDeleteGrid();
	renderGraphicsView();
}

bool PreProcessorGridDataItem::setGrid(Grid* newGrid)
{
		if (m_grid != 0){delete m_grid;}
	m_grid = newGrid;
	// set parent.
	m_grid->setParent(this);
	PreProcessorGraphicsViewInterface* view = dataModel()->graphicsView();
	double xmin, xmax, ymin, ymax;
	view->getDrawnRegion(&xmin, &xmax, &ymin, &ymax);
	m_grid->updateSimplifiedGrid(xmin, xmax, ymin, ymax);
	m_grid->setModified();
	// set zone name.
	QString zname = dynamic_cast<PreProcessorGridAndGridCreatingConditionDataItemInterface*>(parent())->zoneName();
	m_grid->setZoneName(zname);
	if (m_bcGroupDataItem != 0){
		m_bcGroupDataItem->setGrid(newGrid);
		m_bcGroupDataItem->clearPoints();
	}

	// update vtk pipeline.
	finishGridLoading();

	// update the object browser tree structure.
	updateObjectBrowserTree();
	updateActionStatus();
	informGridChange();
	return true;
}

void PreProcessorGridDataItem::informSelection(VTKGraphicsView* v)
{
	m_shapeDataItem->informSelection(v);
}

void PreProcessorGridDataItem::setSelectedPointsVisibility(bool visible)
{
	m_selectedVerticesActor->VisibilityOff();
	m_actorCollection->RemoveItem(m_selectedVerticesActor);
	if (visible){
		m_actorCollection->AddItem(m_selectedVerticesActor);
	}
	updateVisibilityWithoutRendering();
}

void PreProcessorGridDataItem::setSelectedCellsVisibility(bool visible)
{
	m_selectedCellsActor->VisibilityOff();
	m_selectedCellsLinesActor->VisibilityOff();
	m_actorCollection->RemoveItem(m_selectedCellsActor);
	m_actorCollection->RemoveItem(m_selectedCellsLinesActor);
	if (visible){
		m_actorCollection->AddItem(m_selectedCellsActor);
		m_actorCollection->AddItem(m_selectedCellsLinesActor);
	}
	updateVisibilityWithoutRendering();
}

void PreProcessorGridDataItem::setSelectedEdgesVisibility(bool visible)
{
	m_selectedEdgesActor->VisibilityOff();
	m_actorCollection->RemoveItem(m_selectedEdgesActor);
	if (visible){
		m_actorCollection->AddItem(m_selectedEdgesActor);
	}
	updateVisibilityWithoutRendering();
}

void PreProcessorGridDataItem::informDeselection(VTKGraphicsView* v)
{
	m_shapeDataItem->informDeselection(v);
}

void PreProcessorGridDataItem::clearSelection()
{
	m_selectedVertices.clear();
	m_selectedCells.clear();
	m_selectedEdges.clear();

	updateSelectedVerticesGraphics();
	updateSelectedCellsGraphics();
	updateSelectedEdgesGraphics();
}

void PreProcessorGridDataItem::updateSelectedVertices(MouseBoundingBox* box, bool xOr)
{
	m_selectedVertices = getSelectedVertices(box, xOr);

	updateSelectedVerticesGraphics();

	m_shapeDataItem->updateActionStatus();
	updateActionStatus();
	informSelectedVerticesChanged();
}

void PreProcessorGridDataItem::updateSelectedVerticesGraphics()
{
	m_selectedVerticesPolyData->Reset();
	if (m_grid == 0){
		m_selectedVerticesPolyData->BuildCells();
		m_selectedVerticesPolyData->BuildLinks();
		m_selectedVerticesPolyData->Modified();
		return;
	}
	m_selectedVerticesPolyData->SetPoints(m_grid->vtkGrid()->GetPoints());
	vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
	vtkIdType node;
	for (int i = 0; i < m_selectedVertices.count(); ++i){
		node = m_selectedVertices.at(i);
		cells->InsertNextCell(1, &(node));
	}
	m_selectedVerticesPolyData->SetVerts(cells);
	m_selectedVerticesPolyData->BuildCells();
	m_selectedVerticesPolyData->BuildLinks();
	m_selectedVerticesPolyData->Modified();
}

QVector<vtkIdType> PreProcessorGridDataItem::getSelectedVertices(MouseBoundingBox* box, bool xOr)
{
	QVector<vtkIdType> ret;
	if (m_grid == 0){
		// grid is not setup yet.
		return ret;
	}
//	if (m_grid->isMasked()){return ret;}
	QSet<vtkIdType> selectedVerticesSet;
	if (xOr){
		for (vtkIdType i = 0; i < m_selectedVertices.count(); ++i){
			selectedVerticesSet.insert(m_selectedVertices.at(i));
		}
	}

	vtkPoints* points = m_grid->vtkGrid()->GetPoints();
	double p[3];
	Structured2DGrid* sgrid = dynamic_cast<Structured2DGrid*>(m_grid);
	if (sgrid != 0){
		for (int ii = sgrid->drawnIMin(); ii <= sgrid->drawnIMax(); ++ii){
			for (int jj = sgrid->drawnJMin(); jj <= sgrid->drawnJMax(); ++jj){
				vtkIdType idx = sgrid->vertexIndex(ii, jj);
				points->GetPoint(idx, p);
				if (box->isInsideBox(p[0], p[1])){
					if (selectedVerticesSet.contains(idx)){
						selectedVerticesSet.remove(idx);
					} else {
						selectedVerticesSet.insert(idx);
					}
				}
			}
		}
	} else {
		vtkIdType i;
		for (i = 0; i < points->GetNumberOfPoints(); ++i){
			points->GetPoint(i, p);
			if (box->isInsideBox(p[0], p[1])){
				if (selectedVerticesSet.contains(i)){
					selectedVerticesSet.remove(i);
				} else {
					selectedVerticesSet.insert(i);
				}
			}
		}
	}

	for (auto s_it = selectedVerticesSet.begin(); s_it != selectedVerticesSet.end(); ++s_it){
		ret.append(*s_it);
	}
	return ret;
}


QSet<vtkIdType> PreProcessorGridDataItem::getSelectedVerticesSet(MouseBoundingBox* box, bool xOr)
{
	QVector<vtkIdType> vec = getSelectedVertices(box, xOr);
	QSet<vtkIdType> set;
	for (int i = 0; i < vec.size(); ++i){
		set.insert(vec.at(i));
	}
	return set;
}

void PreProcessorGridDataItem::setSelectedVertices(const QVector<vtkIdType>& vertices)
{
	m_selectedVertices = vertices;
	updateSelectedVerticesGraphics();
}

void PreProcessorGridDataItem::informSelectedVerticesChanged()
{
	m_nodeGroupDataItem->informSelectedVerticesChanged(m_selectedVertices);
}

void PreProcessorGridDataItem::updateSelectedCells(MouseBoundingBox* box, bool xOr)
{
	if (m_grid == 0){
		// grid is not setup yet.
		return;
	}
//	if (m_grid->isMasked()){return;}

	bool click = false;
	if (isNear(box->startPoint(), box->endPoint())){
		QPoint newStart = box->endPoint() - QPoint(2, 2);
		QPoint newEnd = box->endPoint() + QPoint(2, 2);
		box->setStartPoint(newStart.x(), newStart.y());
		box->setEndPoint(newEnd.x(), newEnd.y());
		click = true;
	}

	QSet<vtkIdType> selectedCellsSet;
	if (xOr){
		for (vtkIdType i = 0; i < m_selectedCells.count(); ++i){
			selectedCellsSet.insert(m_selectedCells.at(i));
		}
	}
	m_selectedCells.clear();

	QVector<vtkIdType> selectedCellsVector;

	if (click){
		double point[3];
		box->vtkGrid()->GetPoint(0, point);
		// find the cell that contains point.
		vtkCell* hintCell = 0;
		double pcoords[4];
		double weights[4];
		int subid;
		vtkIdType cellid = m_grid->vtkGrid()->FindCell(point, hintCell, 0, 1e-4, subid, pcoords, weights);
		if (cellid >= 0){
			selectedCellsVector.append(cellid);
		}
	} else {
		// setup selectedVertices.
		QSet<vtkIdType> selectedVertices = getSelectedVerticesSet(box, false);
		selectedCellsVector = getCellsFromVertices(selectedVertices);
	}
	for (int i = 0; i < selectedCellsVector.count(); ++i){
		vtkIdType cellid = selectedCellsVector.at(i);
		if (selectedCellsSet.contains(cellid)){
			selectedCellsSet.remove(cellid);
		} else {
			selectedCellsSet.insert(cellid);
		}
	}

	for (auto s_it = selectedCellsSet.begin(); s_it != selectedCellsSet.end(); ++s_it){
		m_selectedCells.append(*s_it);
	}
	updateSelectedCellsGraphics();
	updateActionStatus();
}

void PreProcessorGridDataItem::updateSelectedCellsGraphics()
{

	vtkSmartPointer<vtkIdList> cellids = vtkSmartPointer<vtkIdList>::New();
	for (int i = 0; i < m_selectedCells.count(); ++i){
		cellids->InsertNextId(m_selectedCells[i]);
	}
	m_selectedCellsGrid->SetCellList(cellids);

	m_selectedCellsGrid->Modified();
	m_shapeDataItem->updateActionStatus();
}

void PreProcessorGridDataItem::updateSelectedEdges(MouseBoundingBox* box, bool xOr, VTKGraphicsView* v)
{
	if (m_grid == 0){
		// grid is not setup yet.
		return;
	}
//	if (m_grid->isMasked()){return;}

	bool click = false;
	if (isNear(box->startPoint(), box->endPoint())){
		QPoint center;
		center.setX((box->startPoint().x() + box->endPoint().x()) / 2);
		center.setY((box->startPoint().y() + box->endPoint().y()) / 2);
		box->setStartPoint(center.x(), center.y());
		box->setEndPoint(center.x(), center.y());
		click = true;
	}

	QSet<Edge> selectedEdgesSet;
	if (xOr){
		for (int i = 0; i < m_selectedEdges.count(); ++i){
			selectedEdgesSet.insert(m_selectedEdges.at(i));
		}
	}

	QVector<Edge> selectedEdgesVector;
	if (click){
		double p[3];
		VTK2DGraphicsView* v2 = dynamic_cast<VTK2DGraphicsView*>(v);
		double stdDist = v2->stdRadius(5);
		vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
		box->vtkGrid()->GetPoint(0, p);
		vtkPolyData* pd = buildEdges();
		vtkIdType pointId = pd->FindPoint(p);
		pd->GetPointCells(pointId, cellIds);
		for (vtkIdType i = 0; i < cellIds->GetNumberOfIds(); ++i){
			vtkCell* cell = pd->GetCell(cellIds->GetId(i));
			Edge edge(cell->GetPointId(0), cell->GetPointId(1));
			// check whether cell is clicked.
			double tmpp[3];
			pd->GetPoint(cell->GetPointId(0), tmpp);
			QVector2D v0(tmpp[0], tmpp[1]);
			pd->GetPoint(cell->GetPointId(1), tmpp);
			QVector2D v1(tmpp[0], tmpp[1]);

			QVector2D horizontal = v1 - v0;
			QVector2D vertical = (v1 - v0).normalized() * stdDist;
			iRIC::rotateVector90(vertical);
			QVector2D posv = v0 - vertical * 0.5;
			QVector2D point(p[0], p[1]);
			if (iRIC::isInsideParallelogram(point, posv, horizontal, vertical)){
				selectedEdgesVector.append(edge);
				break;
			}
		}
		pd->Delete();
	} else {
		// setup selectedVertices.
		QSet<vtkIdType> selectedVertices = getSelectedVerticesSet(box, false);
		selectedEdgesVector = getEdgesFromVertices(selectedVertices);
	}
	for (int i = 0; i < selectedEdgesVector.count(); ++i){
		const Edge& e = selectedEdgesVector.at(i);
		if (selectedEdgesSet.contains(e)){
			selectedEdgesSet.remove(e);
		} else {
			selectedEdgesSet.insert(e);
		}
	}

	m_selectedEdges.clear();
	for (auto s_it = selectedEdgesSet.begin(); s_it != selectedEdgesSet.end(); ++s_it){
		m_selectedEdges.append(*s_it);
	}

	updateSelectedEdgesGraphics();

	m_shapeDataItem->updateActionStatus();
	updateActionStatus();
}

void PreProcessorGridDataItem::updateSelectedEdgesGraphics()
{
	m_selectedEdgesPolyData->Reset();
	if (m_grid == 0){
		return;
	}
	m_selectedEdgesPolyData->SetPoints(m_grid->vtkGrid()->GetPoints());
	vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
	vtkIdType nodes[2];
	for (int i = 0; i < m_selectedEdges.count(); ++i){
		const Edge& edge = m_selectedEdges.at(i);
		nodes[0] = edge.vertex1();
		nodes[1] = edge.vertex2();
		cells->InsertNextCell(2, &(nodes[0]));
	}

	m_selectedEdgesPolyData->SetLines(cells);
	m_selectedEdgesPolyData->Modified();
}

void PreProcessorGridDataItem::mouseDoubleClickEvent(QMouseEvent* /*event*/, VTKGraphicsView* /*v*/)
{}

void PreProcessorGridDataItem::mouseMoveEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	m_shapeDataItem->mouseMoveEvent(event, v);
}

void PreProcessorGridDataItem::mousePressEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	m_shapeDataItem->mousePressEvent(event, v);

}

void PreProcessorGridDataItem::mouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v)
{
	m_shapeDataItem->mouseReleaseEvent(event, v);
}

void PreProcessorGridDataItem::nodeSelectingMouseMoveEvent(QMouseEvent* event, VTKGraphicsView* /*v*/)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	// drawing bounding box using mouse dragging.
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setEndPoint(event->x(), event->y());

	renderGraphicsView();
}

void PreProcessorGridDataItem::nodeSelectingMousePressEvent(QMouseEvent* event, VTKGraphicsView* v)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setStartPoint(event->x(), event->y());
	box->enable();

	v->GetRenderWindow()->SetDesiredUpdateRate(PreProcessorDataItem::dragUpdateRate);
	renderGraphicsView();
}

void PreProcessorGridDataItem::nodeSelectingMouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setEndPoint(event->x(), event->y());

	if (isNear(box->startPoint(), box->endPoint())){
		QPoint newStart = box->endPoint() - QPoint(2, 2);
		QPoint newEnd = box->endPoint() + QPoint(2, 2);
		box->setStartPoint(newStart.x(), newStart.y());
		box->setEndPoint(newEnd.x(), newEnd.y());
	}

	bool xOr = ((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier);

	updateSelectedVertices(box, xOr);
	box->disable();

	v->restoreUpdateRate();
	renderGraphicsView();
}

void PreProcessorGridDataItem::nodeSelectingKeyPressEvent(QKeyEvent* event, VTKGraphicsView* v)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	if ((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier){
		v->setCursor(m_addCursor);
		m_shiftPressed = true;
	}else{
		m_shiftPressed = false;
	}
}

void PreProcessorGridDataItem::nodeSelectingKeyReleaseEvent(QKeyEvent* event, VTKGraphicsView* v)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	if ((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier){
		v->setCursor(m_addCursor);
		m_shiftPressed = true;
	}else{
		v->unsetCursor();
		m_shiftPressed = false;
	}
}

void PreProcessorGridDataItem::cellSelectingMouseMoveEvent(QMouseEvent* event, VTKGraphicsView* /*v*/)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	// drawing bounding box using mouse dragging.
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setEndPoint(event->x(), event->y());

	renderGraphicsView();
}

void PreProcessorGridDataItem::cellSelectingMousePressEvent(QMouseEvent* event, VTKGraphicsView* v)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setStartPoint(event->x(), event->y());
	box->enable();

	v->GetRenderWindow()->SetDesiredUpdateRate(PreProcessorDataItem::dragUpdateRate);
	renderGraphicsView();
}

void PreProcessorGridDataItem::cellSelectingMouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setEndPoint(event->x(), event->y());
	bool xOr = ((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier);

	updateSelectedCells(box, xOr);
	box->disable();

	v->restoreUpdateRate();
	renderGraphicsView();
}

void PreProcessorGridDataItem::edgeSelectingMouseMoveEvent(QMouseEvent* event, VTKGraphicsView* /*v*/)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	// drawing bounding box using mouse dragging.
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setEndPoint(event->x(), event->y());

	renderGraphicsView();
}

void PreProcessorGridDataItem::edgeSelectingMousePressEvent(QMouseEvent* event, VTKGraphicsView* v)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setStartPoint(event->x(), event->y());
	box->enable();

	v->GetRenderWindow()->SetDesiredUpdateRate(PreProcessorDataItem::dragUpdateRate);
	renderGraphicsView();
}

void PreProcessorGridDataItem::edgeSelectingMouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v)
{
//	if (m_grid != 0 && m_grid->isMasked()){return;}
	MouseBoundingBox* box = dataModel()->mouseBoundingBox();
	box->setEndPoint(event->x(), event->y());
	bool xOr = ((event->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier);

	updateSelectedEdges(box, xOr, v);
	box->disable();

	v->restoreUpdateRate();
	renderGraphicsView();
}

void PreProcessorGridDataItem::setupActors()
{
	vtkProperty* prop;

	m_regionPolyData = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPoints> tmppoints = vtkSmartPointer<vtkPoints>::New();
	m_regionPolyData->SetPoints(tmppoints);

	m_regionMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_regionMapper->SetInputData(m_regionPolyData);

	m_regionActor = vtkSmartPointer<vtkActor>::New();
	m_regionActor->SetMapper(m_regionMapper);
	prop = m_regionActor->GetProperty();
	prop->SetOpacity(0);
	prop->SetColor(0, 0, 0);
	m_regionActor->VisibilityOff();
	renderer()->AddActor(m_regionActor);

	m_selectedVerticesPolyData = vtkSmartPointer<vtkPolyData>::New();

	m_selectedVerticesActor = vtkSmartPointer<vtkActor>::New();
	prop = m_selectedVerticesActor->GetProperty();
	prop->SetPointSize(5);
	prop->SetLighting(false);
	prop->SetColor(0, 0, 0);
	prop->SetRepresentationToPoints();
	renderer()->AddActor(m_selectedVerticesActor);

	m_selectedVerticesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_selectedVerticesMapper->SetScalarVisibility(false);
	m_selectedVerticesActor->SetMapper(m_selectedVerticesMapper);
	m_selectedVerticesMapper->SetInputData(m_selectedVerticesPolyData);

	m_selectedVerticesActor->VisibilityOff();

	m_selectedCellsGrid = vtkSmartPointer<vtkExtractCells>::New();

	m_selectedCellsActor = vtkSmartPointer<vtkActor>::New();
	prop = m_selectedCellsActor->GetProperty();
	prop->SetLighting(false);
	prop->SetColor(0, 0, 0);
	prop->SetOpacity(0.5);
	prop->SetRepresentationToSurface();

	renderer()->AddActor(m_selectedCellsActor);

	m_selectedCellsMapper = vtkSmartPointer<vtkDataSetMapper>::New();
	m_selectedCellsMapper->SetScalarVisibility(false);
	m_selectedCellsActor->SetMapper(m_selectedCellsMapper);
	m_selectedCellsMapper->SetInputConnection(m_selectedCellsGrid->GetOutputPort());

	m_selectedCellsLinesActor = vtkSmartPointer<vtkActor>::New();
	prop = m_selectedCellsLinesActor->GetProperty();
	prop->SetLighting(false);
	prop->SetLineWidth(3);
	prop->SetColor(0, 0, 0);
	prop->SetRepresentationToWireframe();

	renderer()->AddActor(m_selectedCellsLinesActor);

	m_selectedCellsLinesMapper = vtkSmartPointer<vtkDataSetMapper>::New();
	m_selectedCellsLinesMapper->SetScalarVisibility(false);
	m_selectedCellsLinesActor->SetMapper(m_selectedCellsLinesMapper);
	m_selectedCellsLinesMapper->SetInputConnection(m_selectedCellsGrid->GetOutputPort());

	m_selectedCellsActor->VisibilityOff();
	m_selectedCellsLinesActor->VisibilityOff();

	m_selectedEdgesPolyData = vtkSmartPointer<vtkPolyData>::New();

	m_selectedEdgesActor = vtkSmartPointer<vtkActor>::New();
	prop = m_selectedEdgesActor->GetProperty();
	prop->SetLighting(false);
	prop->SetLineWidth(3);
	prop->SetColor(0, 0, 0);

	renderer()->AddActor(m_selectedEdgesActor);

	m_selectedEdgesMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	m_selectedEdgesMapper->SetScalarVisibility(false);
	m_selectedEdgesActor->SetMapper(m_selectedEdgesMapper);
	m_selectedEdgesMapper->SetInputData(m_selectedEdgesPolyData);

	m_selectedEdgesActor->VisibilityOff();
}

void PreProcessorGridDataItem::setupActions()
{
	m_importAction = new QAction(tr("&Import..."), this);
	m_importAction->setIcon(QIcon(":/libs/guibase/images/iconImport.png"));
	connect(m_importAction, SIGNAL(triggered()), this, SLOT(importGrid()));

	m_exportAction = new QAction(tr("&Export..."), this);
	m_exportAction->setIcon(QIcon(":/libs/guibase/images/iconExport.png"));
	connect(m_exportAction, SIGNAL(triggered()), this, SLOT(exportGrid()));

	m_displaySettingAction = new QAction(tr("Grid &Shape..."), this);
	connect(m_displaySettingAction, SIGNAL(triggered()), this, SLOT(showDisplaySettingDialog()));

	m_polygonSelectAction = new QAction(tr("&Select Polygon Region"), this);

	m_deleteAction = new QAction(tr("&Delete..."), this);
	m_deleteAction->setIcon(QIcon(":/libs/guibase/images/iconDeleteItem.png"));
	connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(deleteGrid()));

	m_menu = new QMenu(tr("&Grid"));
	m_nodeEditAction = new QAction(tr("&Node Attribute..."), this);
	m_nodeDisplaySettingAction = new QAction(tr("&Node Attribute..."), this);

	m_cellEditAction = new QAction(tr("&Cell Attribute..."), this);
	m_cellDisplaySettingAction = new QAction(tr("&Cell Attribute..."), this);

	m_setupScalarBarAction = new QAction(tr("Set &Up Scalarbar..."), this);
	PreProcessorGridTypeDataItem* gtItem = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent());
	connect(m_setupScalarBarAction, SIGNAL(triggered()), gtItem->rawdataTop(), SLOT(setupScalarBar()));

	m_birdEyeWindowAction = new QAction(tr("Open &Bird's-Eye View Window"), this);
	m_birdEyeWindowAction->setIcon(QIcon(":/libs/pre/images/iconBirdEyeWindow.png"));
	connect(m_birdEyeWindowAction, SIGNAL(triggered()), this, SLOT(openBirdEyeWindow()));
}

void PreProcessorGridDataItem::updateZDepthRangeItemCount()
{
	PreProcessorDataItem::updateZDepthRangeItemCount();
	m_zDepthRange.setItemCount(m_zDepthRange.itemCount() + 1);
}

void PreProcessorGridDataItem::informgridRelatedConditionChangeAll()
{
	if (m_grid == 0){return;}
	QList<GridRelatedConditionContainer*> conds = m_grid->gridRelatedConditions();
	for (auto it = conds.begin(); it != conds.end(); ++it){
		informgridRelatedConditionChange((*it)->name());
	}
}

void PreProcessorGridDataItem::informgridRelatedConditionChange(const QString& name)
{
	emit gridRelatedConditionChanged(name);
	PreProcessorGridRelatedConditionNodeDataItem* nItem = m_nodeGroupDataItem->nodeDataItem(name);
	if (nItem != 0){nItem->updateCrossectionWindows();}
}

void PreProcessorGridDataItem::finishGridLoading()
{
	if (m_grid != 0){
		m_selectedVerticesPolyData->SetPoints(m_grid->vtkGrid()->GetPoints());
		m_selectedCellsGrid->SetInputData(m_grid->vtkGrid());
		m_selectedEdgesPolyData->SetPoints(m_grid->vtkGrid()->GetPoints());
	} else {
		m_selectedVerticesPolyData->SetPoints(0);
		m_selectedCellsGrid->SetInputData(0);
		m_selectedEdgesPolyData->SetPoints(0);
	}
	// inform that all grid attributes are updated.
	informgridRelatedConditionChangeAll();
	// update vtk pipeline.
	m_shapeDataItem->informGridUpdate();
	m_nodeGroupDataItem->informGridUpdate();
	m_cellGroupDataItem->informGridUpdate();
	updateRegionPolyData();
}

bool PreProcessorGridDataItem::isImportAvailable()
{
	Grid* tmpgrid = dynamic_cast<PreProcessorGridTypeDataItem*>(parent()->parent())->gridType()->emptyGrid();
	const QList<GridImporterInterface*> importerList = GridImporterFactory::instance().list(tmpgrid->gridType());
	return importerList.count() > 0;
}

bool PreProcessorGridDataItem::isExportAvailable()
{
	if (m_grid == 0){return false;}
	const QList<GridExporterInterface*> exporterList = GridExporterFactory::instance().list(m_grid->gridType());
	return exporterList.count() > 0;
}

QAction* PreProcessorGridDataItem::mappingAction()
{
	PreProcessorGridAndGridCreatingConditionDataItem* item =
			dynamic_cast<PreProcessorGridAndGridCreatingConditionDataItem*>(parent());
	PreProcessorGridAttributeMappingSettingTopDataItem* mItem = item->mappingSettingDataItem();
	return mItem->customMappingAction();
}

void PreProcessorGridDataItem::updateActionStatus()
{
	m_importAction->setEnabled(isImportAvailable());
	m_exportAction->setEnabled(m_grid != 0 && isExportAvailable());

	m_deleteAction->setEnabled(m_grid != 0);
	m_displaySettingAction->setEnabled(m_grid != 0);
	m_polygonSelectAction->setEnabled(m_grid != 0);
	m_birdEyeWindowAction->setEnabled(m_grid != 0);

	m_nodeEditAction->setEnabled((m_grid != 0) && (m_selectedVertices.count() > 0) && (m_nodeDataItem != 0));
	m_nodeDisplaySettingAction->setEnabled((m_grid != 0) && (m_nodeDataItem != 0));
	m_cellEditAction->setEnabled((m_grid != 0) && (m_selectedCells.count() > 0) && (m_cellDataItem != 0));
	m_cellDisplaySettingAction->setEnabled((m_grid != 0) && (m_cellDataItem != 0));

	m_setupScalarBarAction->setEnabled(m_grid != 0);

	m_shapeDataItem->editAction()->setEnabled(m_grid != 0 && m_selectedVertices.count() > 0);

	PreProcessorGridAttributeMappingSettingTopDataItem* mtItem =
			dynamic_cast<PreProcessorGridAndGridCreatingConditionDataItem*>(parent())->mappingSettingDataItem();
	mtItem->customMappingAction()->setEnabled(m_grid != 0);
}

void PreProcessorGridDataItem::silentDeleteGrid()
{
	if (m_grid == 0){return;}
	if (m_bcGroupDataItem != 0){
		m_bcGroupDataItem->clearPoints();
	}
	delete m_grid;
	m_grid = 0;
	updateObjectBrowserTree();
	updateActionStatus();
	finishGridLoading();
	clearSelection();
	closeBirdEyeWindow();
	iRICUndoStack::instance().clear();
}

void PreProcessorGridDataItem::updateAttributeActorSettings()
{
	m_nodeGroupDataItem->updateActorSettings();
	m_cellGroupDataItem->updateActorSettings();
}

QCursor PreProcessorGridDataItem::normalCursor()
{
	if (m_shiftPressed){
		return m_addCursor;
	}else{
		return QCursor(Qt::ArrowCursor);
	}
}

void PreProcessorGridDataItem::openBirdEyeWindow()
{
	QWidget* w;
	if (m_birdEyeWindow == 0){
		m_birdEyeWindow = new GridBirdEyeWindow(iricMainWindow(), this);
		QMdiArea* cent = dynamic_cast<QMdiArea*>(iricMainWindow()->centralWidget());
		w = cent->addSubWindow(m_birdEyeWindow);
		PreProcessorWindowInterface* pre = preProcessorWindow();
		QPoint p = pre->pos() + QPoint(10, 10);
		w->setWindowIcon(m_birdEyeWindow->icon());
		w->move(p);
		w->resize(640, 480);
	} else {
		w = dynamic_cast<QWidget*>(m_birdEyeWindow->parent());
	}
	w->show();
	w->setFocus();
	m_birdEyeWindow->cameraFit();
}

void PreProcessorGridDataItem::closeBirdEyeWindow()
{
	if (m_birdEyeWindow == 0){return;}
	delete m_birdEyeWindow->parent();
}

void PreProcessorGridDataItem::informGridChange()
{
	if (m_birdEyeWindow != 0){
		m_birdEyeWindow->updateGrid();
	}
	clearSelection();
}

void PreProcessorGridDataItem::informBirdEyeWindowClose()
{
	m_birdEyeWindow = 0;
}


void PreProcessorGridDataItem::assignActionZValues(const ZDepthRange& range)
{
	// selected.
	m_regionActor->SetPosition(0, 0, range.min());
	m_selectedVerticesActor->SetPosition(0, 0, range.max());
	m_selectedCellsActor->SetPosition(0, 0, range.max());
	m_selectedCellsLinesActor->SetPosition(0, 0, range.max());

	ZDepthRange r;
	// Boundary Condition
	if (m_bcGroupDataItem != 0){
		r = m_bcGroupDataItem->zDepthRange();
		r.setMax(range.max() * 0.9 + range.min() * 0.1);
		r.setMin(range.max() * 0.8 + range.min() * 0.2);
		m_bcGroupDataItem->setZDepthRange(r);
	}
	// Shape
	r = m_shapeDataItem->zDepthRange();
	r.setMax(range.max() * 0.7 + range.min() * 0.3);
	r.setMin(range.max() * 0.5 + range.min() * 0.5);
	m_shapeDataItem->setZDepthRange(r);

	// Node Condition
	r = m_nodeGroupDataItem->zDepthRange();
	r.setMax(range.max() * 0.4 + range.min() * 0.6);
	r.setMin(range.max() * 0.3 + range.min() * 0.7);
	m_nodeGroupDataItem->setZDepthRange(r);

	// Cell Condition
	r = m_cellGroupDataItem->zDepthRange();
	r.setMax(range.max() * 0.2 + range.min() * 0.8);
	r.setMin(range.max() * 0.1 + range.min() * 0.9);
	m_cellGroupDataItem->setZDepthRange(r);
}

vtkPolyData* PreProcessorGridDataItem::buildEdges() const
{
	if (m_grid == 0){return 0;}
	vtkPolyData* polyData = vtkPolyData::New();
	polyData->SetPoints(m_grid->vtkGrid()->GetPoints());
	QSet<Edge> edges;

	for (vtkIdType i = 0; i < m_grid->vtkGrid()->GetNumberOfCells(); ++i){
		vtkCell* cell = m_grid->vtkGrid()->GetCell(i);
		int edgeCount = cell->GetNumberOfEdges();
		for (int j = 0; j < edgeCount; ++j){
			vtkCell* edgeCell = cell->GetEdge(j);
			Edge e(edgeCell->GetPointId(0), edgeCell->GetPointId(1));
			edges.insert(e);
		}
	}

	vtkSmartPointer<vtkCellArray> ca = vtkSmartPointer<vtkCellArray>::New();
	for (auto it = edges.begin(); it != edges.end(); ++it){
		vtkIdType nodes[2];
		nodes[0] = it->vertex1();
		nodes[1] = it->vertex2();
		ca->InsertNextCell(2, &(nodes[0]));
	}
	polyData->SetLines(ca);
	polyData->BuildCells();
	polyData->BuildLinks();
	return polyData;
}

void PreProcessorGridDataItem::updateObjectBrowserTree()
{
	QStandardItem* sItem = m_shapeDataItem->standardItem();
	if (sItem->row() != - 1){
		// remove.
		m_standardItem->takeRow(sItem->row());
	}
	sItem = m_nodeGroupDataItem->standardItem();
	if (sItem->row() != - 1){
		// remove.
		m_standardItem->takeRow(sItem->row());
	}
	sItem = m_cellGroupDataItem->standardItem();
	if (sItem->row() != - 1){
		// remove.
		m_standardItem->takeRow(sItem->row());
	}
	if (m_bcGroupDataItem){
		sItem = m_bcGroupDataItem->standardItem();
		if (sItem->row() != - 1){
			// remove.
			m_standardItem->takeRow(sItem->row());
		}
	}

	QString cap = tr("Grid");
	if (m_grid == 0){
		// do nothing.
		cap.append(tr(" [No Data]"));
		m_standardItem->setText(cap);
	} else {
		vtkPointSet* ps = m_grid->vtkGrid();
		vtkStructuredGrid* sg = dynamic_cast<vtkStructuredGrid*>(ps);
		vtkUnstructuredGrid* ug = dynamic_cast<vtkUnstructuredGrid*>(ps);

		if (sg != 0){
			int dim[3];
			sg->GetDimensions(dim);
			cap.append(tr(" (%1 x %2 = %3)").arg(dim[0]).arg(dim[1]).arg(dim[0] * dim[1]));
		} else if (ug != 0){
			int num = ug->GetNumberOfPoints();
			cap.append(tr(" (%1)").arg(num));
		}

		m_standardItem->setText(cap);
		// add rows.
		// shape item.
		m_standardItem->appendRow(m_shapeDataItem->standardItem());
		// add node group if node data exists.
		if (m_nodeGroupDataItem->conditions().count() > 0){
			m_standardItem->appendRow(m_nodeGroupDataItem->standardItem());
		}
		// add cell group if node data exists.
		if (m_cellGroupDataItem->conditions().count() > 0){
			m_standardItem->appendRow(m_cellGroupDataItem->standardItem());
		}
		// add boundary condition group if boundary condition is defined.
		if (m_bcGroupDataItem){
			m_standardItem->appendRow(m_bcGroupDataItem->standardItem());
		}
	}
}

QVector<vtkIdType> PreProcessorGridDataItem::getCellsFromVertices(const QSet<vtkIdType> &vertices) const
{
	QSet<vtkIdType> selectedCellNoms;
	vtkSmartPointer<vtkIdList> idlist = vtkSmartPointer<vtkIdList>::New();
	for (auto v_it = vertices.begin(); v_it != vertices.end(); ++v_it){
		m_grid->vtkGrid()->GetPointCells(*v_it, idlist);
		for (vtkIdType i = 0; i < idlist->GetNumberOfIds(); ++i){
			vtkIdType cellid = idlist->GetId(i);
			selectedCellNoms.insert(cellid);
		}
	}

	QVector<vtkIdType> ret;
	for (auto v_it = selectedCellNoms.begin(); v_it != selectedCellNoms.end(); ++v_it){
		bool allSelected = true;
		m_grid->vtkGrid()->GetCellPoints(*v_it, idlist);
		for (vtkIdType i = 0; i < idlist->GetNumberOfIds(); ++i){
			vtkIdType pointid = idlist->GetId(i);
			allSelected = allSelected && vertices.contains(pointid);
		}
		if (allSelected){
			ret.append(*v_it);
		}
	}
	qSort(ret);
	return ret;
}

QVector<Edge> PreProcessorGridDataItem::getEdgesFromVertices(const QSet<vtkIdType> &vertices) const
{
	QSet<Edge> selectedEdgeNoms;

	vtkPolyData* pd = buildEdges();

	vtkSmartPointer<vtkIdList> idlist = vtkSmartPointer<vtkIdList>::New();
	for (auto v_it = vertices.begin(); v_it != vertices.end(); ++v_it){
		pd->GetPointCells(*v_it, idlist);
		for (vtkIdType i = 0; i < idlist->GetNumberOfIds(); ++i){
			vtkIdType cellid = idlist->GetId(i);
			vtkCell* cell = pd->GetCell(cellid);
			Edge edge(cell->GetPointId(0), cell->GetPointId(1));
			selectedEdgeNoms.insert(edge);
		}
	}

	QVector<Edge> ret;
	for (auto e_it = selectedEdgeNoms.begin(); e_it != selectedEdgeNoms.end(); ++e_it){
		if (vertices.contains(e_it->vertex1()) && vertices.contains(e_it->vertex2())){
			ret.append(*e_it);
		}
	}
	qSort(ret);
	return ret;
}

void PreProcessorGridDataItem::doViewOperationEndedGlobal(VTKGraphicsView *v)
{
	updateSimplefiedGrid(v);
}

void PreProcessorGridDataItem::updateSimplefiedGrid(VTKGraphicsView *v)
{
	if (m_grid == 0){return;}
	if (v == 0){
		v = dataModel()->graphicsView();
	}
	PreProcessorGraphicsView* view = dynamic_cast<PreProcessorGraphicsView*> (v);
	double xmin, xmax, ymin, ymax;
	view->getDrawnRegion(&xmin, &xmax, &ymin, &ymax);
	m_grid->updateSimplifiedGrid(xmin, xmax, ymin, ymax);
/*
	if (m_grid->isMasked()){
		clearSelection();
	}
*/

	m_shapeDataItem->informGridUpdate();
	m_nodeGroupDataItem->informGridUpdate();
	m_cellGroupDataItem->informGridUpdate();
}

void PreProcessorGridDataItem::updateRegionPolyData()
{
	Grid* grid = this->grid();
	if (grid == 0){
		m_regionPolyData->Reset();
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
		m_regionPolyData->SetPoints(points);
		return;
	}
	double bounds[6];
	grid->vtkGrid()->GetBounds(bounds);

	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	points->Allocate(4);
	points->InsertNextPoint(bounds[0], bounds[2], 0);
	points->InsertNextPoint(bounds[1], bounds[2], 0);
	points->InsertNextPoint(bounds[1], bounds[3], 0);
	points->InsertNextPoint(bounds[0], bounds[3], 0);
	m_regionPolyData->SetPoints(points);

	vtkIdType pts[4] = {0, 1, 2, 3};
	vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
	cells->InsertNextCell(4, pts);
	m_regionPolyData->SetPolys(cells);
	m_regionPolyData->Modified();
	actorCollection()->RemoveItem(m_regionActor);
	actorCollection()->AddItem(m_regionActor);
	updateVisibilityWithoutRendering();
}

void PreProcessorGridDataItem::doApplyOffset(double x, double y)
{
	if (m_grid == 0){return;}
	vtkPoints* points = m_grid->vtkGrid()->GetPoints();
	vtkIdType numPoints = points->GetNumberOfPoints();
	for (vtkIdType id = 0; id < numPoints; ++id){
		double v[3];
		points->GetPoint(id, v);
		v[0] -= x;
		v[1] -= y;
		points->SetPoint(id, v);
	}
	points->Modified();
	m_grid->setModified();
	this->updateRegionPolyData();
}
