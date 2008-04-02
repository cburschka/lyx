
#include "LayoutEditor.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	lyx::MainWindow led;
	QStringList args = app.arguments();
	args.append("/data/lyx/trunk/lib/layouts/amsart.layout");
	for (int i = 1; i != args.size(); ++i) {
		led.loadLayoutFile(args[i]);
	}
	led.show();
	return app.exec();
}

