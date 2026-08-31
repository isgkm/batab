#include "batab.h"

#include <QApplication>

// NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
int main(int argc, char *argv[])
{
    const QApplication app(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    const Batab mainWindow;
    // mainWindow.show();
    return QCoreApplication::exec();
}
