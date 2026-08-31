#ifndef BATAB_H
#define BATAB_H

#include "appswitcher.h"

#include <QMainWindow>
#include <QSystemTrayIcon>

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

signals:
    void timerCompleted();

protected:
    void createActions();
    void createTrayIcon();

private:
    Ui::Batab *m_ui;
    static Batab *s_ui;

    QSystemTrayIcon *m_trayIcon{};
    QMenu *m_trayIconMenu{};

    AppSwitcher *m_appSwitcher{};

    QAction *m_minimizeAction{};
    QAction *m_maximizeAction{};
    QAction *m_restoreAction{};
    QAction *m_quitAction{};

    QTimer *m_timer{};
};
#endif // BATAB_H
