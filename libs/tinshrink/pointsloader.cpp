#include "pointsloader.h"

#include <triangle/triangle.h>
#include <triangle/triangleutil.h>

#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <fstream>
#include <vector>

vtkPolyData* PointsLoader::loadPoints(const std::string& name)
{
	std::fstream f(name.c_str(), std::ios::in);

	auto points = vtkPoints::New();
	points->SetDataTypeToDouble();
	auto da = vtkDoubleArray::New();
	da->SetName("value");

	while (! f.eof()) {
		double x, y, z;
		f >> x >> y >> z;

		points->InsertNextPoint(x, y, 0);
		da->InsertNextValue(z);
	}

	auto ret = vtkPolyData::New();
	ret->SetPoints(points);

	auto ca = vtkCellArray::New();
	for (int i = 0; i < points->GetNumberOfPoints(); ++i) {
		vtkIdType id = i;
		ca->InsertNextCell(1, &id);
	}
	ret->SetVerts(ca);
	ret->GetPointData()->AddArray(da);

	return ret;
}

vtkPolyData* PointsLoader::buildTINFromPoints(vtkPolyData* pd)
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

	out.pointlist = in.pointlist;

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

PointsLoader::PointsLoader()
{}
