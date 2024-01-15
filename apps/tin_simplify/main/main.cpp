#include "pointsloader.h"
#include "tinsimplifier.h"

#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	auto writer = vtkSmartPointer<vtkPolyDataWriter>::New();

	auto inputData = PointsLoader::loadPoints("input.tpo");
	writer->SetFileName("input.vtk");
	writer->SetInputData(inputData);
	writer->Update();

	auto tin1 = PointsLoader::buildTINFromPoints(inputData);
	writer->SetFileName("tin1.vtk");
	writer->SetInputData(tin1);
	writer->Update();

	auto contour = TinSimplifier::buildContour(tin1, 1);
	writer->SetFileName("contour.vtk");
	writer->SetInputData(contour);
	writer->Update();

	auto tin2 = TinSimplifier::buildTINFromContour(contour);
	writer->SetFileName("tin2.vtk");
	writer->SetInputData(tin2);
	writer->Update();

	return 0;
}
