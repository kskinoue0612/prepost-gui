#include "pointsloader.h"
#include "tinshrink_main.h"
#include "tinsimplifier.h"

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>

#include <iostream>
#include <string>

int tinshrink_main(const std::string& input, const std::string& output, double interval, double dist_threshold1, double dist_threshold2, double angle_threshold)
{
	int inputNumPoints, outputNumPoints;

	auto tin = tinshrink(input, interval, dist_threshold1, dist_threshold2, angle_threshold, &inputNumPoints, &outputNumPoints);
	if (tin == nullptr) {return -1;}

	auto writer = vtkSmartPointer<vtkSTLWriter>::New();
	writer->SetFileName(output.c_str());
	writer->SetFileTypeToBinary();
	writer->SetInputData(tin);
	writer->Update();

	return 0;
}

vtkPolyData *tinshrink(const std::string& input, double interval, double dist_threshold1, double dist_threshold2, double angle_threshold, int* inputNumPoints, int* outputNumPoints)
{
	auto reader = vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName(input.c_str());
	reader->Update();
	auto tin1 = reader->GetOutput();
	*inputNumPoints = tin1->GetPoints()->GetNumberOfPoints();

	auto points = tin1->GetPoints();
	auto values = vtkSmartPointer<vtkDoubleArray>::New();
	values->SetName("value");
	values->Allocate(points->GetNumberOfPoints());
	double v[3];
	for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i) {
		points->GetPoint(i, v);
		values->InsertNextValue(v[2]);
		v[2] = 0;
		points->SetPoint(i, v);
	}
	tin1->GetPointData()->AddArray(values);

	auto contour = TinSimplifier::buildContour(tin1, interval);
	auto contour2 = TinSimplifier::simplifyContour(contour, interval, dist_threshold1, dist_threshold2, angle_threshold);
	contour->Delete();


	auto tin2 = TinSimplifier::buildTINFromContour(contour2);
	contour2->Delete();

	auto values2 = tin2->GetPointData()->GetArray("value");
	points = tin2->GetPoints();
	for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i) {
		points->GetPoint(i, v);
		v[2] = values2->GetTuple1(i);
		points->SetPoint(i, v);
	}
	*outputNumPoints = points->GetNumberOfPoints();

	return tin2;
}

