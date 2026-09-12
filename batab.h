#ifndef BATAB_H
#define BATAB_H

#include "appswitcher.h"

#include <QElapsedTimer>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
class Batab;
}  // namespace Ui
QT_END_NAMESPACE

class Batab final : public QMainWindow
{
    Q_OBJECT

public:
    explicit Batab(QWidget *parent = nullptr);
    ~Batab() override;

    Q_DISABLE_COPY_MOVE(Batab)

    [[nodiscard]] static Batab *getUI()
    {
        return s_ui;
    }
    [[nodiscard]] AppSwitcher *getAppSwitcher()
    {
        return m_appSwitcher;
    }

protected:
    void createActions();
    void createTrayIcon();
    void checkHotCorner();

private:
    Ui::Batab *m_ui;
    static Batab *s_ui;

    AppSwitcher *m_appSwitcher{};

    QTimer *m_hotCornerCheckTimer{};
    QElapsedTimer m_hotCornerElapsedTime;
    bool m_hotCornerTimerRunning{false};

    QSystemTrayIcon *m_trayIcon{};
    QMenu *m_trayIconMenu{};
    QAction *m_minimizeAction{};
    QAction *m_maximizeAction{};
    QAction *m_restoreAction{};
    QAction *m_quitAction{};
};
#endif // BATAB_H
