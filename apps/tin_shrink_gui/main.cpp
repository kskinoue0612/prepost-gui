#include "tinshrinkguidialog.h"

#include <QString>
#include <QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	TinShrinkGuiDialog dialog;
	dialog.show();

	return a.exec();
}
