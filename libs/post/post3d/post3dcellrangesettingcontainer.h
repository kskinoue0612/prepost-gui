#ifndef POST3DCELLRANGESETTINGCONTAINER_H
#define POST3DCELLRANGESETTINGCONTAINER_H

#include <misc/boolcontainer.h>
#include <misc/compositecontainer.h>
#include <misc/intcontainer.h>

class Post3dCellRangeSettingContainer : public CompositeContainer
{
public:
	Post3dCellRangeSettingContainer();
	Post3dCellRangeSettingContainer(const Post3dCellRangeSettingContainer& c);
	~Post3dCellRangeSettingContainer();

	Post3dCellRangeSettingContainer& operator=(const Post3dCellRangeSettingContainer& c);
	XmlAttributeContainer& operator=(const XmlAttributeContainer& c) override;

	BoolContainer enabled;
	IntContainer iMin;
	IntContainer iMax;
	IntContainer jMin;
	IntContainer jMax;
	IntContainer kMin;
	IntContainer kMax;
};

#endif // POST3DCELLRANGESETTINGCONTAINER_H
