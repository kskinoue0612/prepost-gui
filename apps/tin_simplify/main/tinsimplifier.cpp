#include "tinsimplifier.h"

#include <triangle/triangle.h>
#include <triangle/triangleutil.h>

#include <vtkContourFilter.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

#include <cmath>
#include <set>

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
	for (int i = 0; i < vals.size(); ++i) {
		filter->SetValue(i, vals.at(i));
	}
	filter->SetInputData(input);
	filter->Update();

	auto output = filter->GetOutput();
	output->Register(nullptr);
	return output;
}

vtkPolyData* TinSimplifier::buildTINFromContour(vtkPolyData* pd)
{
	triangulateio in, out;

	TriangleUtil::clearTriangulateio(&in);
	TriangleUtil::clearTriangulateio(&out);

	std::vector<double> pointlist;
	auto points = pd->GetPoints();
	pointlist.reserve(points->GetNumberOfPoints() * 2);
	for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i) {
		double v[3];
		points->GetPoint(i, v);
		pointlist.push_back(v[0]);
		pointlist.push_back(v[1]);
	}

	in.numberofpoints = pointlist.size() / 2;
	in.pointlist = pointlist.data();

	auto lines = pd->GetLines();

	std::vector<int> tmp_seglist;
	std::set<Edge> edges;
	vtkIdType npts;
	vtkIdType *pts = nullptr;
	for (lines->InitTraversal(); lines->GetNextCell(npts, pts); ) {
		for (int j = 0; j < npts - 1; ++j) {
			vtkIdType id1 = *(pts + j);
			vtkIdType id2 = *(pts + j + 1);

			edges.insert(Edge(id1, id2));
		}
	}

	std::vector<int> seglist;
	seglist.reserve(edges.size() * 2);
	for (const auto& edge : edges) {
		seglist.push_back(edge.id1 + 1);
		seglist.push_back(edge.id2 + 1);
	}

	in.numberofsegments = seglist.size() / 2;
	in.segmentlist = seglist.data();

	char triangle_arg[] = "pcj";

	triangulate(triangle_arg, &in, &out, nullptr);

	auto ret = vtkPolyData::New();
	ret->SetPoints(points);

	auto tris = vtkCellArray::New();
	for (int i = 0; i < out.numberoftriangles; ++i) {
		vtkIdType ids[3];
		ids[0] = *(out.trianglelist + i * 3 + 0) - 1;
		ids[1] = *(out.trianglelist + i * 3 + 1) - 1;
		ids[2] = *(out.trianglelist + i * 3 + 2) - 1;
		tris->InsertNextCell(3, ids);
	}
	ret->SetPolys(tris);
	auto da = pd->GetPointData()->GetArray("value");
	ret->GetPointData()->AddArray(da);

	return ret;
}

TinSimplifier::TinSimplifier()
{}
