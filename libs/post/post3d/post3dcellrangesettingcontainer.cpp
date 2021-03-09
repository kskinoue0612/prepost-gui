#include "post3dcellrangesettingcontainer.h"

Post3dCellRangeSettingContainer::Post3dCellRangeSettingContainer() :
	CompositeContainer({&enabled, &iMin, &iMax, &jMin, &jMax, &kMin, &kMax}),
	enabled {"enabled", true},
	iMin {"iMin"},
	iMax {"iMax"},
	jMin {"jMin"},
	jMax {"jMax"},
	kMin {"kMin"},
	kMax {"kMax"}
{
}

Post3dCellRangeSettingContainer::Post3dCellRangeSettingContainer(const Post3dCellRangeSettingContainer& c) :
	Post3dCellRangeSettingContainer()
{
	copyValue(c);
}

Post3dCellRangeSettingContainer::~Post3dCellRangeSettingContainer()
{}

Post3dCellRangeSettingContainer& Post3dCellRangeSettingContainer::operator=(const Post3dCellRangeSettingContainer& c)
{
	copyValue(c);
	return *this;
}

XmlAttributeContainer& Post3dCellRangeSettingContainer::operator=(const XmlAttributeContainer& c)
{
	return operator=(dynamic_cast<const Post3dCellRangeSettingContainer&> (c));
}
