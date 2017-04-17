#ifndef INPUTCONDITIONWIDGETFUNCTIONALDELEGATE_H
#define INPUTCONDITIONWIDGETFUNCTIONALDELEGATE_H

#include <QStyledItemDelegate>

class InputConditionWidgetFunctionalDialog;

class InputConditionWidgetFunctionalDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	InputConditionWidgetFunctionalDelegate(QObject* = nullptr) {}

	void setDialog(InputConditionWidgetFunctionalDialog* dialog);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
	InputConditionWidgetFunctionalDialog* m_dialog;
};

#endif // INPUTCONDITIONWIDGETFUNCTIONALDELEGATE_H
