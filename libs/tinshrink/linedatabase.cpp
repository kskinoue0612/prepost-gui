#include "linedatabase.h"

#include <misc/mathsupport.h>

#include <vtkPoints.h>

#include <QPointF>

#include <geos/geom/Envelope.h>

#include <algorithm>

LineDatabase::LineDatabase(vtkPoints* points, const std::vector<Line>& lines) :
	m_points {points}
{
	for (const auto& line : lines) {
		for (int i = 0; i < static_cast<int>(line.ids.size()) - 1; ++i) {
			Segment seg {line.ids[i], line.ids[i + 1]};
			m_segments.push_back(seg);

			double v1[3], v2[3];
			points->GetPoint(seg.point1, v1);
			points->GetPoint(seg.point2, v2);

			double xmin = std::min(v1[0], v2[0]);
			double xmax = std::max(v1[0], v2[0]);
			double ymin = std::min(v1[1], v2[1]);
			double ymax = std::max(v1[1], v2[1]);

			geos::geom::Envelope env(xmin, xmax, ymin, ymax);
			long long index = m_segments.size() - 1;
			m_qTree.insert(&env, reinterpret_cast<void*>(index));
		}
	}
}

bool LineDatabase::intersect(const QPointF& p1, const QPointF& p2)
{
	double xmin = std::min(p1.x(), p2.x());
	double xmax = std::max(p1.x(), p2.x());
	double ymin = std::min(p1.y(), p2.y());
	double ymax = std::max(p1.y(), p2.y());

	geos::geom::Envelope env(xmin, xmax, ymin, ymax);
	std::vector<void*> ret;
	m_qTree.query(&env, ret);

	QPointF intersection;
	double r, s;

	for (void* vptr: ret) {
		auto idx = reinterpret_cast<long long>(vptr);
		const auto& seg = m_segments.at(idx);
		double v1[3], v2[3];
		m_points->GetPoint(seg.point1, v1);
		m_points->GetPoint(seg.point2, v2);

		QPointF q1(v1[0], v1[1]);
		QPointF q2(v2[0], v2[1]);

		bool intersect = iRIC::intersectionPoint(p1, p2, q1, q2, &intersection, &r, &s);
		if (! intersect) {continue;}
		if (r < 0) {continue;}
		if (r > 1) {continue;}
		if (s < 0) {continue;}
		if (s > 1) {continue;}

		return true;
	}

	return false;
}
