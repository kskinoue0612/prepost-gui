#include "post3dwindowcellcontourgrouptopdataitem_createcommand.h"

Post3dWindowCellContourGroupTopDataItem::CreateCommand::CreateCommand(const ScalarSettingContainer& scalarSetting, const std::vector<Post3dCellRangeSettingContainer>& rangeSettings, Post3dWindowCellContourGroupTopDataItem* item) :
	m_scalarSetting {scalarSetting},
	m_rangeSettings {rangeSettings},
	m_newItem {nullptr},
	m_item {item}
{}

Post3dWindowCellContourGroupTopDataItem::CreateCommand::~CreateCommand()
{}

void Post3dWindowCellContourGroupTopDataItem::CreateCommand::redo()
{
	m_newItem = new Post3dWindowCellContourGroupDataItem(m_item);
	m_newItem->setIsCommandExecuting(true);

	m_newItem->standardItem()->setText(m_scalarSetting.target);
	m_newItem->updateZScale(m_item->m_zScale);
	m_newItem->setSetting(m_scalarSetting, m_rangeSettings);
	m_item->m_childItems.push_back(m_newItem);

	m_newItem->setIsCommandExecuting(false);
	m_item->updateItemMap();
}

void Post3dWindowCellContourGroupTopDataItem::CreateCommand::undo()
{
	delete m_newItem;
	m_newItem = nullptr;

	m_item->update();
	m_item->updateItemMap();
}
