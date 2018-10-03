#ifndef PREPROCESSORGRIDANDGRIDCREATINGCONDITIONDATAITEMINTERFACE_H
#define PREPROCESSORGRIDANDGRIDCREATINGCONDITIONDATAITEMINTERFACE_H

#include "../../guicore_global.h"
#include "preprocessordataitem.h"

#include <string>

class QString;
class PreProcessorGridDataItemInterface;
class PreProcessorGridCreatingConditionDataItemInterface;

class GUICOREDLL_EXPORT PreProcessorGridAndGridCreatingConditionDataItemInterface : public PreProcessorDataItem
{

public:
	PreProcessorGridAndGridCreatingConditionDataItemInterface(const QString& caption, PreProcessorDataItem* parent);
	virtual ~PreProcessorGridAndGridCreatingConditionDataItemInterface();
	virtual const QString& caption() const = 0;
	virtual PreProcessorGridDataItemInterface* gridDataItem() const = 0;
	virtual const std::string& zoneName() const = 0;
	virtual PreProcessorGridCreatingConditionDataItemInterface* creatingConditionDataItem() const = 0;
	virtual bool isGridEdited() const = 0;
};

#endif // PREPROCESSORGRIDANDGRIDCREATINGCONDITIONDATAITEMINTERFACE_H
