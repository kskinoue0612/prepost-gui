#ifndef TINSIMPLIFIER_H
#define TINSIMPLIFIER_H

class vtkPolyData;

class TinSimplifier
{
public:
	static vtkPolyData* buildContour(vtkPolyData* input, double scale);
	static vtkPolyData* buildTINFromContour(vtkPolyData* contour);

private:
	TinSimplifier();
};

#endif // TINSIMPLIFIER_H
