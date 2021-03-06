#ifndef CROSSSECTIONSPREPROCESSORCONTROLLER_IMPL_H
#define CROSSSECTIONSPREPROCESSORCONTROLLER_IMPL_H

#include "../crosssectionspreprocessorcontroller.h"

class QAction;

class CrossSectionsPreProcessorController::Impl
{
public:
	Impl();
	~Impl();

	QAction* m_addAction;
	QAction* m_importAction;
	QAction* m_exportAction;
};

#endif // CROSSSECTIONSPREPROCESSORCONTROLLER_IMPL_H
