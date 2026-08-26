#include "batab.h"

#include "ui_batab.h"
#include "winprocs.h"

#include <QMenu>
#include <QMessageBox>
#include <QTimer>

Batab *Batab::s_ui = nullptr;

Batab::Batab(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Batab)
{
    ui->setupUi(this);
    s_ui = this;
    appSwitcher = new AppSwitcher();

    EnumWindows(WinProcs::enumWindowsProc, NULL);

    timer = new QTimer(this);
    timer->setSingleShot(true);
    QObject::connect(this->timer,
                     &QTimer::timeout,
                     this->appSwitcher,
                     &AppSwitcher::showUIAfterTimerCompleted);

    WinProcs::registerLLKHook();
    WinProcs::registerWEHook();

    createActions();
    createTrayIcon();

    QIcon icon(":/assets/icon.png");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);

    trayIcon->setToolTip("Batab");

    trayIcon->show();
}

Batab::~Batab()
{
    WinProcs::unregisterLLKHook();
    WinProcs::unregisterWEHook();

    s_ui = nullptr;
    delete appSwitcher;
    delete ui;
}

void Batab::createActions()
{
    minimizeAction = new QAction(tr("&Minimize"), this);
    connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

    maximizeAction = new QAction(tr("&Maximize"), this);
    connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Batab::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}