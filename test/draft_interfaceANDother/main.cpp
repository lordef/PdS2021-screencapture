//#include <bits/stdc++.h>
#include "ScreenRecorder.h"
#if QT
#include "QtWidgetsClass.h"
#include <QtWidgets/QApplication>
#endif
using namespace std;

int main(int argc, char* argv[])
{
	/*
	* Crea QT Application e Widget
	*/
	QApplication a(argc, argv);
	QtWidgetsClass w;
	w.setWindowTitle(QString("Vola mio mini recorder"));

	w.show();
	a.exec();
	return 0;
}
