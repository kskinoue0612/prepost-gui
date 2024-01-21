#include "line.h"
#include "linedatabase.h"
#include "tinsimplifier.h"

#include <guibase/polyline/polylineutil.h>
#include <misc/mathsupport.h>
#include <triangle/triangle.h>
#include <triangle/triangleutil.h>

#include <vtkContourFilter.h>
#include <vtkDoubleArray.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

#include <QPointF>
#include <QRectF>

#include <geos/geom/Envelope.h>
#include <geos/index/quadtree/Quadtree.h>

#include <cmath>
#include <set>
#include <unordered_map>

namespace {

int newPointId(std::unordered_map<int, int>* pointMapId, int *nextPointId, int origPointId)
{
	auto it = pointMapId->find(origPointId);
	if (it != pointMapId->end()) {return it->second;}

	int newId = *nextPointId;
	pointMapId->insert({origPointId, newId});
	++ *nextPointId;

	return newId;
}

QPointF getPoint(vtkIdType id, vtkPoints* points)
{
	double v[3];
	points->GetPoint(id, v);

	return QPointF(v[0], v[1]);
}

bool tryRemovePoint(int index, std::vector<vtkIdType>* lineData, vtkPoints* points, double threDistance1, double threDistance2, double cosThreshold, LineDatabase* lineDb)
{
	auto p1 = getPoint(lineData->at(index - 1), points);
	auto p2 = getPoint(lineData->at(index + 1), points);
	auto target = getPoint(lineData->at(index), points);

	auto threDistance2Squared = threDistance2 * threDistance2;

	if (iRIC::lengthSquared(p1 - target) > threDistance2Squared) {return false;}
	if (iRIC::lengthSquared(p2 - target) > threDistance2Squared) {return false;}

	QPointF leg;

	double r = iRIC::perpendicularLineOfLeg(p1, p2, target, &leg);
	if (r < 0 || r > 1) {return false;}

	double dist2 = iRIC::lengthSquared(target - leg);
	if (dist2 > threDistance1 * threDistance1) {return false;}

	auto v1 = p1 - target;
	auto v2 = p2 - target;

	qreal dotprod = QPointF::dotProduct(v1, v2);
	double cosVal = dotprod / (iRIC::length(v1) * iRIC::length(v2));
	if (cosVal > -1 + cosThreshold) {return false;}

	if (lineDb->intersect(p1, p2)) {return false;}

	lineData->erase(lineData->begin() + index);
	return true;
}

std::vector<vtkIdType> simplifyLine(const std::vector<vtkIdType>& lineData, vtkPoints* points, double threDistance1, double threDistance2, double cosThreshold, LineDatabase* lineDb)
{
	auto ret = lineData;

	int index = 1;
	while (index < static_cast<int> (ret.size()) - 1) {
		bool removed = tryRemovePoint(index, &ret, points, threDistance1, threDistance2, cosThreshold, lineDb);

		if (! removed) {++index;}
	}

	return ret;
}

void addToEndIdMap(std::unordered_map<int, std::vector<int> >* endIdMap, int lineId, int endId)
{
	auto it = endIdMap->find(endId);
	if (it == endIdMap->end()) {
		std::vector<int> empty;
		auto pair2 = endIdMap->insert({endId, empty});
		it = pair2.first;
	}
	it->second.push_back(lineId);
}

void removeFromEndIdMap(std::unordered_map<int, std::vector<int> >* endIdMap, int lineId, int endId)
{
	auto it = endIdMap->find(endId);
	if (it == endIdMap->end()) {return;}

	auto it2 = std::find(it->second.begin(), it->second.end(), lineId);
	if (it2 == it->second.end()) {return;}

	it->second.erase(it2);
}

} // namespace

vtkPolyData* TinSimplifier::buildContour(vtkPolyData* input, double scale)
{
	auto da = input->GetPointData()->GetArray("value");
	double range[2];
	da->GetRange(range);

	double min = static_cast<int> (range[0] / scale) * scale;
	double max = (static_cast<int> (range[1] / scale) + 1) * scale;

	std::vector<double> vals;
	double v = min;
	while (v <= max) {
		vals.push_back(v);
		v += scale;
	}
	input->GetPointData()->SetActiveScalars("value");

	auto filter = vtkSmartPointer<vtkContourFilter>::New();
	filter->SetNumberOfContours(vals.size());
	for (int i = 0; i < static_cast<int>(vals.size()); ++i) {
		filter->SetValue(i, vals.at(i));
	}
	filter->SetInputData(input);
	filter->Update();

	auto output = filter->GetOutput();
	output->Register(nullptr);
	return output;
}

vtkPolyData* TinSimplifier::simplifyContour(vtkPolyData* input, double interval, double distThreshold1, double distThreshold2, double angleThreshold)
{
	double cos = std::cos(angleThreshold / 180.0 * 3.1415926535);
	double cosThreshold = cos + 1;
	std::vector<Line> lineVec;

	vtkIdType npts;
	vtkIdType *pts = nullptr;

	auto lines = input->GetLines();
	for (lines->InitTraversal(); lines->GetNextCell(npts, pts); ) {
		Line lineData;
		for (int j = 0; j < npts; ++j) {
			lineData.ids.push_back(*(pts + j));
		}
		lineVec.push_back(lineData);
	}

	std::unordered_map<int, std::vector<int> > endIdMap;
	while (true) {
		int mergedCount = 0;
		std::vector<Line> newLineVec;
		endIdMap.clear();

		for (const auto& lineData : lineVec) {
			bool merged = false;
			vtkIdType oldEnd, newEnd;

			auto it1 = endIdMap.find(*lineData.ids.begin());
			if (it1 != endIdMap.end()) {
				for (auto lineId : it1->second) {
					auto& l = newLineVec[lineId];
					merged = l.merge(lineData, &oldEnd, &newEnd);
					if (merged) {
						removeFromEndIdMap(&endIdMap, lineId, oldEnd);
						addToEndIdMap(&endIdMap, lineId, newEnd);
						break;
					}
				}
			}
			if (merged) {
				++ mergedCount;
				continue;
			}

			auto it2 = endIdMap.find(*lineData.ids.rbegin());
			if (it2 != endIdMap.end()) {
				for (auto lineId : it2->second) {
					auto& l = newLineVec[lineId];
					merged = l.merge(lineData, &oldEnd, &newEnd);
					if (merged) {
						removeFromEndIdMap(&endIdMap, lineId, oldEnd);
						addToEndIdMap(&endIdMap, lineId, newEnd);
						break;
					}
				}
			}
			if (merged) {
				++ mergedCount;
				continue;
			}

			newLineVec.push_back(lineData);
			auto newLineId = newLineVec.size() - 1;
			addToEndIdMap(&endIdMap, newLineId, *lineData.ids.begin());
			addToEndIdMap(&endIdMap, newLineId, *lineData.ids.rbegin());
		}
		lineVec = newLineVec;

		if (mergedCount == 0) {break;}
	}

	auto value = input->GetPointData()->GetArray("value");
	auto points = input->GetPoints();

	auto newLines = vtkSmartPointer<vtkCellArray>::New();
	for (int i = 0; i < static_cast<int> (lineVec.size()); ++i) {
		auto& line = lineVec[i];
		double v = value->GetTuple1(line.ids[0]);

		std::vector<Line> checkTargetLines;
		for (int j = 0; j < static_cast<int> (lineVec.size()); ++j) {
			if (j == i) {continue;}

			const auto& line2 = lineVec[j];
			double v2 = value->GetTuple1(line2.ids[0]);
			if (std::abs(v2 - v) < interval * 0.5) {continue;}

			checkTargetLines.push_back(line2);
		}

		LineDatabase db(input->GetPoints(), checkTargetLines);
		auto simplifiedLineData = simplifyLine(line.ids, points, distThreshold1, distThreshold2, cosThreshold, &db);
		newLines->InsertNextCell(simplifiedLineData.size(), simplifiedLineData.data());
	}

	auto ret = vtkPolyData::New();
	ret->SetPoints(points);
	ret->SetLines(newLines);
	ret->GetPointData()->AddArray(value);

	return ret;
}

vtkPolyData* TinSimplifier::buildTINFromContour(vtkPolyData* pd)
{
	triangulateio in, out;

	TriangleUtil::clearTriangulateio(&in);
	TriangleUtil::clearTriangulateio(&out);

	int nextPointId = 0;
	std::unordered_map<int, int> pointIdMap;

	auto lines = pd->GetLines();

	vtkIdType npts;
	vtkIdType *pts = nullptr;

	std::vector<int> seglist;

	for (lines->InitTraversal(); lines->GetNextCell(npts, pts); ) {
		for (int j = 0; j < npts - 1; ++j) {
			vtkIdType id1 = newPointId(&pointIdMap, &nextPointId, *(pts + j));
			vtkIdType id2 = newPointId(&pointIdMap, &nextPointId, *(pts + j + 1));
			seglist.push_back(id1 + 1);
			seglist.push_back(id2 + 1);
		}
	}

	in.numberofsegments = seglist.size() / 2;
	in.segmentlist = seglist.data();

	auto points = pd->GetPoints();
	std::vector<double> pointlist;
	std::vector<double> values;
	pointlist.reserve(pointIdMap.size() * 2);
	values.reserve(pointIdMap.size());

	std::vector<int> idVec;
	idVec.assign(pointIdMap.size(), 0);

	for (const auto& pair : pointIdMap) {
		idVec[pair.second] = pair.first;
	}


	auto oldValues = pd->GetPointData()->GetArray("value");

	for (int i = 0; i < static_cast<int> (idVec.size()); ++i) {
		double v[3];
		points->GetPoint(idVec[i], v);
		pointlist.push_back(v[0]);
		pointlist.push_back(v[1]);

		double value = oldValues->GetTuple1(idVec[i]);
		values.push_back(value);
	}

	in.numberofpoints = pointlist.size() / 2;
	in.pointlist = pointlist.data();
	in.numberofpointattributes = 1;
	in.pointattributelist = values.data();

	char triangle_arg[] = "pcj";

	triangulate(triangle_arg, &in, &out, nullptr);

	auto ret = vtkPolyData::New();

	auto newPoints = vtkSmartPointer<vtkPoints>::New();
	newPoints->SetDataTypeToDouble();
	auto newValues = vtkSmartPointer<vtkDoubleArray>::New();
	newValues->SetName("value");

	auto pl = out.pointlist;
	auto v = out.pointattributelist;

	for (int i = 0; i < out.numberofpoints; ++i) {
		newPoints->InsertNextPoint(*(pl + i * 2), *(pl + i * 2 + 1), 0);
		newValues->InsertNextValue(*(v + i));
	}

	ret->SetPoints(newPoints);

	auto tris = vtkSmartPointer<vtkCellArray>::New();
	for (int i = 0; i < out.numberoftriangles; ++i) {
		vtkIdType ids[3];
		ids[0] = *(out.trianglelist + i * 3 + 0) - 1;
		ids[1] = *(out.trianglelist + i * 3 + 1) - 1;
		ids[2] = *(out.trianglelist + i * 3 + 2) - 1;
		tris->InsertNextCell(3, ids);
	}
	ret->SetPolys(tris);
	ret->GetPointData()->AddArray(newValues);

	return ret;
}

TinSimplifier::TinSimplifier()
{}
