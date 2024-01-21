#ifndef TINSIMPLIFIER_H
#define TINSIMPLIFIER_H

class vtkPolyData;

class TinSimplifier
{
public:
	static vtkPolyData* buildContour(vtkPolyData* input, double interval);
	static vtkPolyData* simplifyContour(vtkPolyData* input, double interval, double distThreshold1, double distThreshold2, double angleThreshold);
	static vtkPolyData* buildTINFromContour(vtkPolyData* contour);

private:
	TinSimplifier();
};

#endif // TINSIMPLIFIER_H
