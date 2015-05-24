#ifndef POSTZONEDATACONTAINER_H
#define POSTZONEDATACONTAINER_H

#include "../guicore_global.h"
#include "postdatacontainer.h"
#include "../pre/grid/grid.h"

#include <QString>
#include <vtkPointSet.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <cgnslib.h>

class RectRegion;
class SolverDefinitionGridType;
class PostExportSetting;

class GUICOREDLL_EXPORT PostZoneDataContainer : public PostDataContainer
{
public:
	const static QString labelName;
	const static QString IBC;
	const static double IBCLimit;
	PostZoneDataContainer(const QString& baseName, const QString& zoneName, SolverDefinitionGridType* gridtype, ProjectDataItem* parent);
	SolverDefinitionGridType* gridType() const {return m_gridType;}
	vtkPointSet* data() const {return m_data;}
	vtkPointSet* labelData() const {return m_labelData;}
	vtkPolyData* particleData() const {return m_particleData;}
	vtkPolyData* filteredData(double xmin, double xmax, double ymin, double ymax, bool& masked) const;
	int baseId() const {return m_baseId;}
	int zoneId() const {return m_zoneId;}
	bool handleCurrentStepUpdate(const int fn) {
		loadFromCgnsFile(fn);
		return m_loadOK;
	}
	void loadFromCgnsFile(const int fn);
	const QString zoneName() const {return m_zoneName;}
	/// Caption is the region name in pre-processor.
	/// Currently, zone name is used instead, temporally.
	const QString caption() const {return zoneName();}
	bool scalarValueExists() const;
	bool vectorValueExists() const;
	int nodeIndex(int i, int j, int k) const;
	void getNodeIJKIndex(int index, int* i, int* j, int* k) const;
	int cellIndex(int i, int j, int k) const;
	void getCellIJKIndex(int index, int* i, int* j, int* k) const;
	bool saveToVTKFile(const QString& filename, double time, const PostExportSetting &s);
	bool saveToCSVFile(const QString& filename, double time, const PostExportSetting &s);
	void loadIfEmpty(const int fn);
	bool IBCExists();
	const QString elevationName();
protected:
	bool setBaseId(const int fn);
	bool setZoneId(const int fn);
	bool loadZoneSize(const int fn);
	virtual bool loadStructuredGrid(const int fn, const int currentStep);
	bool loadUnstructuredGrid(const int fn, const int currentStep);
	bool loadParticle(const int fn, const int currentStep);
	bool getSoluionId(const int fn, const int currentStep, int* solid);
	virtual bool loadScalarData(const int fn, const int solid);
	virtual bool loadVectorData(const int fn, const int solid);
	bool loadCellFlagData(const int fn);
	bool setupIndexData();
	void doLoadFromProjectMainFile(const QDomNode& /*node*/) {}
	void doSaveToProjectMainFile(QXmlStreamWriter& /*writer*/) {}

	vtkPolyData* filteredDataStructured(double xmin, double xmax, double ymin, double ymax, bool& masked) const;
	vtkPolyData* filteredDataUnstructured(double xmin, double xmax, double ymin, double ymax, bool& masked) const;
	int lineLimitI(int j, int iIn, int iOut, const RectRegion& region) const;
	int lineLimitJ(int i, int jIn, int jOut, const RectRegion& region) const;
	int lineLimitI2(int iIn, int iOut, const RectRegion& region) const;
	int lineLimitJ2(int jIn, int jOut, const RectRegion& region) const;
	bool lineAtIIntersect(int i, const RectRegion& region) const;
	bool lineAtJIntersect(int j, const RectRegion& region) const;

protected:
	SolverDefinitionGridType* m_gridType;
	vtkSmartPointer<vtkPointSet> m_data;
	vtkSmartPointer<vtkPointSet> m_labelData;
	vtkSmartPointer<vtkPolyData> m_particleData;
	QString m_baseName;
	QString m_zoneName;
	int m_baseId;
	int m_zoneId;
	int m_cellDim;
	cgsize_t m_sizes[9];

	bool m_loadOK;
	bool m_loadedOnce;
};

#endif // POSTZONEDATACONTAINER_H
