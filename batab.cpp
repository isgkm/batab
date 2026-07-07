#include "batab.h"
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include "ui_batab.h"
#include "winprocs.h"
#include <dwmapi.h>
#include <windows.h>

Batab *Batab::s_ui = nullptr;

bool Batab::isAltTabWindow(const HWND hWnd)
{
    if (GetWindowTextLengthW(hWnd) == 0) {
        return false;
    }

    // if (hWnd == GetShellWindow()) {
    //     return false;
    // }

    if (!IsWindowVisible(hWnd)) {
        return false;
    }

    if (GetAncestor(hWnd, GA_ROOT) != hWnd) {
        return false;
    }

    // LONG style = GetWindowLongW(hWnd, GWL_STYLE);
    // if (style & WS_DISABLED) {
    //     return TRUE;
    // }

    LONG exStyle = GetWindowLongW(hWnd, GWL_EXSTYLE);
    if ((exStyle & WS_EX_TOOLWINDOW) && !(exStyle & WS_EX_APPWINDOW)) {
        return false;
    }

    DWORD cloaked = FALSE;
    HRESULT result = DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (result == S_OK && cloaked == DWM_CLOAKED_SHELL) {
        return false;
    }

    return true;
}

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

    // hookLowLevelKeyboard = SetWindowsHookExW(WH_KEYBOARD_LL,
    //                                          WinProcs::lowLevelKeyboardProc,
    //                                          GetModuleHandle(NULL),
    //                                          0);

    // hookWindowsEvent = SetWinEventHook(EVENT_OBJECT_CREATE,
    //                                    EVENT_OBJECT_UNCLOAKED,
    //                                    NULL,
    //                                    WinProcs::winEventProc,
    //                                    0,
    //                                    0,
    //                                    WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

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
    // UnhookWindowsHookEx(hookLowLevelKeyboard);
    // UnhookWinEvent(hookWindowsEvent);
    WinProcs::unregisterLLKHook();
    WinProcs::unregisterWEHook();

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