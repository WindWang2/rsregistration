
#include <QApplication>

#include "imagewindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	//Q_INIT_RESOURCE(dockwidgets);
	ImageWindow mainWin;
	mainWin.show();
	return app.exec();
}
