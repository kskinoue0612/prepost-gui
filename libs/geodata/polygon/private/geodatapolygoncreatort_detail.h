#ifndef GEODATAPOLYGONCREATORT_DETAIL_H
#define GEODATAPOLYGONCREATORT_DETAIL_H

#include "../geodatapolygoncreatort.h"
#include "../geodatapolygoncellmappert.h"
#include "../geodatapolygonnodemappert.h"

template <class V, class DA>
GeoDataPolygonCreatorT<V, DA>::GeoDataPolygonCreatorT(const QString& typeName) :
	GeoDataPolygonCreator {typeName}
{
	nodeMappers().append(new GeoDataPolygonNodeMapperT<V, DA>(this));
	cellMappers().append(new GeoDataPolygonCellMapperT<V, DA>(this));
}

template <class V, class DA>
GeoDataPolygonCreatorT<V, DA>::~GeoDataPolygonCreatorT()
{}

template <class V, class DA>
bool GeoDataPolygonCreatorT<V, DA>::isCompatibleWith(SolverDefinitionGridAttribute* condition) const
{
	auto cond = dynamic_cast<SolverDefinitionGridAttributeT<V>* >(condition);
	if (cond == nullptr) {return false;}
	if (condition->dimensions().size() > 0) {return false;}

	return true;
}

#endif // GEODATAPOLYGONCREATORT_DETAIL_H