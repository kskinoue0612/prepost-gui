#ifndef PREPROCESSORBCDATAITEM_H
#define PREPROCESSORBCDATAITEM_H

#include <guicore/pre/base/preprocessordataitem.h>
#include <misc/edge.h>

#include <QString>
#include <QColor>
#include <QSet>

class SolverDefinition;
class SolverDefinitionBoundaryCondition;
class QAction;
class VTKGraphicsView;
class Grid;

class PreProcessorBCDataItem : public PreProcessorDataItem
{
	Q_OBJECT

public:
	PreProcessorBCDataItem(SolverDefinition* def, SolverDefinitionBoundaryCondition* cond, GraphicsWindowDataItem* parent, bool hideSetting = false);
	~PreProcessorBCDataItem() override;

	void loadFromCgnsFile(const int fn) override;
	void saveToCgnsFile(const int fn) override;
	void handleStandardItemDoubleClicked() override;
	void handleStandardItemChange() override;

	void setName(const QString& name);

	void setProjectNumber(int num);
	int projectNumber() const;

	void setCgnsNumber(int num);
	int cgnsNumber() const;

	void setColor(const QColor& color);
	QColor color() const;

	int opacityPercent() const;
	SolverDefinitionBoundaryCondition* condition() const;
	bool isValid() const;
	std::string uniqueName() const;

	void informSelection(VTKGraphicsView* v) override;
	void informDeselection(VTKGraphicsView* v) override;
	void mouseMoveEvent(QMouseEvent* event, VTKGraphicsView* v) override;
	void mousePressEvent(QMouseEvent* event, VTKGraphicsView* v) override;
	void mouseReleaseEvent(QMouseEvent* event, VTKGraphicsView* v) override;
	void keyPressEvent(QKeyEvent*, VTKGraphicsView*) override;
	void keyReleaseEvent(QKeyEvent*, VTKGraphicsView*) override;
	void addCustomMenuItems(QMenu* menu) override;
	void clearPoints();
	void assignIndices(const QSet<vtkIdType>& vertices);
	bool isCustomModified() const;
	QString caption() const;
	void setMapped(bool mapped);
	bool mapped() const;

	QAction* editAction() const;
	QAction* deleteAction() const;
	QAction* assignAction() const;
	QAction* releaseAction() const;
	bool hideSetting() const;

protected:
	void doLoadFromProjectMainFile(const QDomNode& node) override;
	void doSaveToProjectMainFile(QXmlStreamWriter& writer) override;
	void doApplyOffset(double x, double y) override;
	virtual void loadExternalData(const QString& filename) override;
	virtual void saveExternalData(const QString& filename) override;

public slots:
	bool showDialog();

private slots:
	void setModified(bool modified = true) override;
	void assignSelectedElements();
	void releaseSelectedElements();

signals:
	void itemUpdated();

protected:
	void assignActorZValues(const ZDepthRange& range) override;

private:
	void setupActors();
	void updateActorSettings();
	void updateNameActorSettings();
	void updateElements();
	int buildNumber() const;

	class Impl;
	Impl* impl;
};

#ifdef _DEBUG
	#include "private/preprocessorbcdataitem_impl.h"
#endif // _DEBUG

#endif // PREPROCESSORBCDATAITEM_H
