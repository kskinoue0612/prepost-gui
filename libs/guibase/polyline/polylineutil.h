#ifndef POLYLINEUTIL_H
#define POLYLINEUTIL_H

#include "../guibase_global.h"

#include <vector>

class vtkPoints;

class QRectF;
class QPointF;

class GUIBASEDLL_EXPORT PolyLineUtil
{
public:
	static std::vector<QPointF> buildSplinePoints(vtkPoints* points, int divNum);
	static double length(const std::vector<QPointF>& polyLine);
	static QRectF boundingRect(const std::vector<QPointF>& polyLine);
	static bool intersects(const std::vector<QPointF>& line1, const std::vector<QPointF>& line2);

private:
	PolyLineUtil();
};

#endif // POLYLINEUTIL_H
