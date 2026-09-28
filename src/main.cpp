#include "batab.h"
#include "constants.h"

#include <QApplication>

// NOLINTNEXTLINE(modernize-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
int main(int argc, char* argv[]) {
    const QApplication app(argc, argv);

    QCoreApplication::setApplicationName("Batab");
    QCoreApplication::setApplicationVersion(Constants::BATAB_VERSION);

    QApplication::setQuitOnLastWindowClosed(false);

    Batab batab;
    batab.show();

    return QCoreApplication::exec();
}
