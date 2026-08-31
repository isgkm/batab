#include "batab.h"

#include "ui_batab.h"
#include "winprocs.h"

#include <QMenu>
#include <QMessageBox>
#include <QTimer>

Batab *Batab::s_ui = nullptr;

Batab::Batab(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::Batab)
    , m_appSwitcher(new AppSwitcher())
    , m_timer(new QTimer(this))
{
    m_ui->setupUi(this);
    s_ui = this;

    EnumWindows(WinProcs::enumWindowsProc, 0);

    m_timer->setSingleShot(true);
    QObject::connect(this->m_timer, &QTimer::timeout, this->m_appSwitcher,
                     &AppSwitcher::showUIAfterTimerCompleted);

    WinProcs::registerLLKHook();
    WinProcs::registerWEHooks();

    createActions();
    createTrayIcon();

    const QIcon icon(":/assets/icon.png");
    m_trayIcon->setIcon(icon);
    setWindowIcon(icon);

    m_trayIcon->setToolTip("Batab");

    m_trayIcon->show();
}

Batab::~Batab()
{
    WinProcs::unregisterLLKHook();
    WinProcs::unregisterWEHooks();

    s_ui = nullptr;
    delete m_appSwitcher;
    delete m_ui;
}

void Batab::createActions()
{
    m_minimizeAction = new QAction(tr("&Minimize"), this);
    connect(m_minimizeAction, &QAction::triggered, this, &QWidget::hide);

    m_maximizeAction = new QAction(tr("&Maximize"), this);
    connect(m_maximizeAction, &QAction::triggered, this,
            &QWidget::showMaximized);

    m_restoreAction = new QAction(tr("&Restore"), this);
    connect(m_restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    m_quitAction = new QAction(tr("&Quit"), this);
    connect(m_quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Batab::createTrayIcon()
{
    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->addAction(m_minimizeAction);
    m_trayIconMenu->addAction(m_maximizeAction);
    m_trayIconMenu->addAction(m_restoreAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);
}