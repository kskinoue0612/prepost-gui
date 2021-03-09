#ifndef POST3DWINDOWCELLCONTOURGROUPTOPDATAITEM_H
#define POST3DWINDOWCELLCONTOURGROUPTOPDATAITEM_H

#include "../post3dwindowdataitem.h"

#include <QMap>

class vtkActor;
class vtkDataSetMapper;

class Post3dWindowCellContourGroupTopDataItem : public Post3dWindowDataItem
{
	Q_OBJECT

public:
	Post3dWindowCellContourGroupTopDataItem(Post3dWindowDataItem* p);
	~Post3dWindowCellContourGroupTopDataItem();

	void innerUpdateZScale(double scale) override;
	void update();

private:
	void addCustomMenuItems(QMenu* menu) override;
	QDialog* addDialog(QWidget* p) override;
	void handleAddDialogAccepted(QDialog* propDialog) override;
	void doLoadFromProjectMainFile(const QDomNode&) override;
	void doSaveToProjectMainFile(QXmlStreamWriter&) override;

	QMap<std::string, QString> m_colorBarTitleMap;
	double m_zScale;

	friend class Post3dWindowCellContourGroupDataItem;

	class CreateCommand;

	QAction* m_addAction;


	vtkActor* m_actor;
	vtkDataSetMapper* m_mapper;
};

#endif // POST3DWINDOWCELLCONTOURGROUPTOPDATAITEM_H
