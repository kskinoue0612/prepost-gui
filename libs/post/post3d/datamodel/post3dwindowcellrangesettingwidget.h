#ifndef POST3DWINDOWCELLRANGESETTINGWIDGET_H
#define POST3DWINDOWCELLRANGESETTINGWIDGET_H

#include <QWidget>

namespace Ui {
class Post3dWindowCellRangeSettingWidget;
}

class PostZoneDataContainer;

class Post3dWindowCellRangeSettingWidget : public QWidget
{
	Q_OBJECT

public:
	class Setting {
	public:
		bool enabled;
		unsigned int iMin;
		unsigned int iMax;
		unsigned int jMin;
		unsigned int jMax;
		unsigned int kMin;
		unsigned int kMax;
	};

	explicit Post3dWindowCellRangeSettingWidget(QWidget *parent = nullptr);
	~Post3dWindowCellRangeSettingWidget();

	void setZoneData(PostZoneDataContainer* zd);

	void setSetting(const Setting& setting);
	Setting setting() const;

private slots:
	void iMinChanged(int imin);
	void iMaxChanged(int imax);
	void jMinChanged(int jmin);
	void jMaxChanged(int jmax);
	void kMinChanged(int kmin);
	void kMaxChanged(int kmax);

private:
	Ui::Post3dWindowCellRangeSettingWidget *ui;
};

#endif // POST3DWINDOWCELLRANGESETTINGWIDGET_H
