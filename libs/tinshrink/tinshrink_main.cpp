#include "pointsloader.h"
#include "tinshrink_main.h"
#include "tinsimplifier.h"

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>

#include <iostream>
#include <string>

int tinshrink_main(const std::string& input, const std::string& output, double interval, double dist_threshold, double angle_threshold)
{
	auto reader = vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName(input.c_str());
	reader->Update();
	auto tin1 = reader->GetOutput();

	auto contour = TinSimplifier::buildContour(tin1, interval);
	auto contour2 = TinSimplifier::simplifyContour(contour, dist_threshold, angle_threshold);
	auto tin2 = TinSimplifier::buildTINFromContour(contour2);

	auto writer = vtkSmartPointer<vtkSTLWriter>::New();
	writer->SetFileName(output.c_str());
	writer->SetInputData(tin2);
	writer->Update();

	return 0;
}
