#include "MainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    try
    {
        w.show();
        a.exec();
    }

    catch (std::runtime_error& re)
    {
        QMessageBox messageBox;
        messageBox.critical(0, "Error", QString::fromStdString(re.what()));
        messageBox.showNormal();
        exit(EXIT_FAILURE);
        //cerr << re.what() << endl;
        return -1;
    }
    //...
    return 0; 

}
