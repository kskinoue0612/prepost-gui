#include "referencemarkpreprocessorviewhelper.h"
#include "../points/pointsgraphicssetting.h"

#include <QColor>

ReferenceMarkPreProcessorViewHelper::ReferenceMarkPreProcessorViewHelper(DataItemView* v) :
	PointsPreProcessorViewHelper {v}
{}

void ReferenceMarkPreProcessorViewHelper::draw(QPainter* painter) const
{
	auto setting = PointsGraphicsSetting::waterElevationPointsSetting;
	drawTriangles(STD_SIZE, Qt::black, setting.transparency, painter);
}
