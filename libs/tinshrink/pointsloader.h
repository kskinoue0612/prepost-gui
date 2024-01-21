#ifndef POINTSLOADER_H
#define POINTSLOADER_H

#include <string>

class vtkPolyData;

class PointsLoader
{
public:
	static vtkPolyData* loadPoints(const std::string& name);
	static vtkPolyData* buildTINFromPoints(vtkPolyData* points);

private:
	PointsLoader();
};

#endif // POINTSLOADER_H
