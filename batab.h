#ifndef BATAB_H
#define BATAB_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "appswitcher.h"
#include <windows.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class Batab;
}
QT_END_NAMESPACE

class Batab final : public QMainWindow
{
    Q_OBJECT

public:
    explicit Batab(QWidget *parent = nullptr);
    ~Batab() override;

    // TODO: can this be avoided with signals and slots?
    [[nodiscard]] inline static Batab *getUI() { return s_ui; }
    [[nodiscard]] inline AppSwitcher *getAppSwitcher() { return appSwitcher; }
    static bool isAltTabWindow(const HWND hWnd);

signals:
    void timerCompleted();

protected:
    void createActions();
    void createTrayIcon();

private:
    Ui::Batab *ui;
    static Batab *s_ui;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    AppSwitcher *appSwitcher;

    QAction *minimizeAction;
    QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QTimer *timer;
};
#endif // BATAB_H
