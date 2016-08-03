#include "reg_test.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QStringList paths = QCoreApplication::libraryPaths();
	paths.append(".");
	//paths.append("./plugins");
	QCoreApplication::setLibraryPaths(paths);

	QApplication a(argc, argv);
	reg_test w;
	w.show();
	return a.exec();
}
