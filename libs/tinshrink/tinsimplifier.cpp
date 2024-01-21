#include "tinsimplifier.h"

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

#include <cmath>
#include <set>
#include <unordered_map>

namespace {

class Line {
public:

	bool merge(const Line& line, vtkIdType* oldEnd, vtkIdType* newEnd)
	{
		if (*ids.rbegin() == *line.ids.begin()) {
			*oldEnd = *ids.rbegin();
			*newEnd = *line.ids.rbegin();
			auto it = line.ids.begin() + 1;
			while (it != line.ids.end()) {
				ids.push_back(*it);
				++ it;
			}
			return true;
		} else if (*ids.rbegin() == *line.ids.rbegin()) {
			*oldEnd = *ids.rbegin();
			*newEnd = *line.ids.begin();
			auto it = line.ids.rbegin() + 1;
			while (it != line.ids.rend()) {
				ids.push_back(*it);
				++ it;
			}
			return true;
		} else if (*ids.begin() == *line.ids.begin()) {
			*oldEnd = *ids.begin();
			*newEnd = *line.ids.rbegin();
			std::reverse(ids.begin(), ids.end());
			auto it = line.ids.begin() + 1;
			while (it != line.ids.end()) {
				ids.push_back(*it);
				++ it;
			}
			return true;
		} else if (*ids.begin() == *line.ids.rbegin()) {
			*oldEnd = *ids.begin();
			*newEnd = *line.ids.begin();
			std::reverse(ids.begin(), ids.end());
			auto it = line.ids.rbegin() + 1;
			while (it != line.ids.rend()) {
				ids.push_back(*it);
				++ it;
			}
			return true;
		}

		return false;
	}

	std::vector<vtkIdType> ids;
};

struct Edge {
	Edge(vtkIdType i1, vtkIdType i2) :
		id1 {std::min(i1, i2)}, id2 {std::max(i1, i2)}
	{}

	bool operator<(const Edge& e) const {
		if (id1 != e.id1) {
			return id1 < e.id1;
		}

		return id2 < e.id2;
	}

	vtkIdType id1;
	vtkIdType id2;
};

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

bool tryRemovePoint(int index, std::vector<vtkIdType>* lineData, vtkPoints* points, double threDistance, double cosThreshold)
{
	auto p1 = getPoint(lineData->at(index - 1), points);
	auto p2 = getPoint(lineData->at(index + 1), points);
	auto target = getPoint(lineData->at(index), points);

	QPointF leg;

	double r = iRIC::perpendicularLineOfLeg(p1, p2, target, &leg);
	if (r < 0 || r > 1) {return false;}

	double dist2 = iRIC::lengthSquared(target - leg);
	if (dist2 > threDistance * threDistance) {return false;}

	auto v1 = p1 - target;
	auto v2 = p2 - target;

	qreal dotprod = QPointF::dotProduct(v1, v2);
	double cosVal = dotprod / (iRIC::length(v1) * iRIC::length(v2));
	if (cosVal > -1 + cosThreshold) {return false;}

	lineData->erase(lineData->begin() + index);
	return true;
}

std::vector<vtkIdType> simplifyLine(const std::vector<vtkIdType>& lineData, vtkPoints* points, double threDistance, double cosThreshold)
{
	auto ret = lineData;

	int index = 1;
	while (index < ret.size() - 1) {
		bool removed = tryRemovePoint(index, &ret, points, threDistance, cosThreshold);

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

vtkPolyData* TinSimplifier::simplifyContour(vtkPolyData* input, double distThreshold, double angleThreshold)
{
	double cos = std::cos(angleThreshold / 180.0 * 3.1415926535);
	double cosThreshold = cos + 1;
	std::vector<Line> lineVec;
	std::unordered_map<int, std::vector<int> > endIdMap;

	vtkIdType npts;
	vtkIdType *pts = nullptr;

	auto lines = input->GetLines();
	for (lines->InitTraversal(); lines->GetNextCell(npts, pts); ) {
		Line lineData;
		for (int j = 0; j < npts; ++j) {
			lineData.ids.push_back(*(pts + j));
		}

		bool merged = false;
		vtkIdType oldEnd, newEnd;

		auto it1 = endIdMap.find(*lineData.ids.begin());
		if (it1 != endIdMap.end()) {
			for (auto lineId : it1->second) {
				auto& l = lineVec[lineId];
				merged = l.merge(lineData, &oldEnd, &newEnd);
				if (merged) {
					removeFromEndIdMap(&endIdMap, lineId, oldEnd);
					addToEndIdMap(&endIdMap, lineId, newEnd);
					break;
				}
			}
		}
		if (merged) {continue;}

		auto it2 = endIdMap.find(*lineData.ids.rbegin());
		if (it2 != endIdMap.end()) {
			for (auto lineId : it2->second) {
				auto& l = lineVec[lineId];
				merged = l.merge(lineData, &oldEnd, &newEnd);
				if (merged) {
					removeFromEndIdMap(&endIdMap, lineId, oldEnd);
					addToEndIdMap(&endIdMap, lineId, newEnd);
					break;
				}
			}
		}
		if (merged) {continue;}

		lineVec.push_back(lineData);
		auto newLineId = lineVec.size() - 1;
		addToEndIdMap(&endIdMap, newLineId, *lineData.ids.begin());
		addToEndIdMap(&endIdMap, newLineId, *lineData.ids.rbegin());
	}
	auto ret = vtkPolyData::New();

	ret->SetPoints(input->GetPoints());
	auto value = input->GetPointData()->GetArray("value");
	ret->GetPointData()->AddArray(value);

	auto newLines = vtkSmartPointer<vtkCellArray>::New();
	for (auto& line : lineVec) {
		auto simplifiedLineData = simplifyLine(line.ids, input->GetPoints(), distThreshold, cosThreshold);
		newLines->InsertNextCell(simplifiedLineData.size(), simplifiedLineData.data());
	}
	ret->SetLines(newLines);

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

bool TinSimplifier::checkContourCross(vtkPolyData* contour)
{
	return true;
}

TinSimplifier::TinSimplifier()
{}
