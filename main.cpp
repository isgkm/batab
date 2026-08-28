#include "batab.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    Batab w;
    // w.show();
    return QCoreApplication::exec();
}
