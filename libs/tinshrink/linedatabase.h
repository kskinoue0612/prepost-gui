#ifndef LINEDATABASE_H
#define LINEDATABASE_H

#include "line.h"

#include <geos/index/quadtree/Quadtree.h>

#include <vector>

class vtkPoints;
class QPointF;

class LineDatabase
{
public:
	LineDatabase(vtkPoints* points, const std::vector<Line>& lines);
	bool intersect(const QPointF& p1, const QPointF& p2);

private:
	struct Segment {
		vtkIdType point1;
		vtkIdType point2;
	};
	std::vector<Segment> m_segments;
	vtkPoints* m_points;
	geos::index::quadtree::Quadtree m_qTree;
};

#endif // LINEDATABASE_H
