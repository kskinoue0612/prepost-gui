#include "tinshrink_api.h"

#include <string>

class vtkPolyData;

int TINSHRINK_API tinshrink_main(const std::string& input, const std::string& output, double interval, double dist_threshold1, double dist_threshold2, double angle_threshold);
vtkPolyData TINSHRINK_API *tinshrink(const std::string& input, double interval, double dist_threshold1, double dist_threshold2, double angle_threshold, int* inputNumPoints, int* outputNumPoints);
