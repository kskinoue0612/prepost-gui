#include "tinshrink_api.h"

#include <string>

class vtkPolyData;

int TINSHRINK_API tinshrink_main(const std::string& input, const std::string& output, double interval, double dist_threshold, double angle_threshold);
vtkPolyData TINSHRINK_API *tinshrink(const std::string& input, double interval, double dist_threshold, double angle_threshold, int* inputNumPoints, bool *crossOk, int* outputNumPoints);
