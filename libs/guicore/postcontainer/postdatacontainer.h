#ifndef POSTDATACONTAINER_H
#define POSTDATACONTAINER_H

#include "../guicore_global.h"
#include "../project/projectdataitem.h"

class PostSolutionInfo;

class GUICOREDLL_EXPORT PostDataContainer : public ProjectDataItem
{
	Q_OBJECT

public:
	PostDataContainer(ProjectDataItem* parent);
	virtual bool handleCurrentStepUpdate(const int fn);
	PostSolutionInfo* postSolutionInfo() const;

signals:
	void dataUpdated();
};

#endif // POSTDATACONTAINER_H
