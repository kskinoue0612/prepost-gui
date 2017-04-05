#include "gridcreatingconditionpoisson.h"

#include "private/gridcreatingconditionpoisson_impl.h"

GridCreatingConditionPoisson::GridCreatingConditionPoisson(ProjectDataItem* parent, GridCreatingConditionCreator* creator) :
	GridCreatingCondition(parent, creator),
	impl {new Impl {}}
{}

GridCreatingConditionPoisson::~GridCreatingConditionPoisson()
{
	delete impl;
}

bool GridCreatingConditionPoisson::create(QWidget* parent)
{
	return true;
}

bool GridCreatingConditionPoisson::ready() const
{
	return true;
}

void GridCreatingConditionPoisson::clear()
{}

void GridCreatingConditionPoisson::doLoadFromProjectMainFile(const QDomNode& node)
{
/*
	QDomNode generatorNode = iRIC::getChildNode(node, "Poisson");
	if (! generatorNode.isNull()) {
		loadPoissonFromProjectMainFile(generatorNode);
	}
*/
}

void GridCreatingConditionPoisson::doSaveToProjectMainFile(QXmlStreamWriter& writer)
{
/*
	writer.writeStartElement("Poisson");
	savePoissonToProjectMainFile(writer);
	writer.writeEndElement();
*/
}

