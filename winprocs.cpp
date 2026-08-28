#include "winprocs.h"

#include "appswitcher.h"
#include "batab.h"
#include "trackedwindows.h"
#include "util.h"

#include <QDebug>

HHOOK WinProcs::hookLowLevelKeyboard{nullptr};
HWINEVENTHOOK WinProcs::hookWinAppLifecycleEvent{nullptr};
HWINEVENTHOOK WinProcs::hookWinAppNameChangeEvent{nullptr};

bool WinProcs::isLLKHooked{};
bool WinProcs::areWEHooksActive{};

BOOL CALLBACK WinProcs::enumWindowsProc(HWND hWnd, LPARAM lparam)
{
    int length = GetWindowTextLengthW(hWnd);
    std::wstring windowTitle;
    windowTitle.resize(length);

    GetWindowTextW(hWnd, windowTitle.data(), windowTitle.size() + 1);

    if (!Util::isAltTabWindow(hWnd))
    {
        return TRUE;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(hWnd, &processId);

    TrackedWindows::getInstance()->addWindow(hWnd,
                                             {.title = QString::fromStdWString(windowTitle),
                                              .PID = processId,
                                              .icon = Util::getIconFromHWND(hWnd)});

    return TRUE;
}

LRESULT CALLBACK WinProcs::lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *) lParam;

        if (p->vkCode == VK_TAB) {
            bool altDown = p->flags & LLKHF_ALTDOWN;

            if (altDown && wParam == WM_SYSKEYDOWN) {
                qDebug() << "alt+tab detected";

                keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0);

                Batab* ui = Batab::getUI();
                if(ui && ui->getAppSwitcher()){
                    QMetaObject::invokeMethod(ui->getAppSwitcher(), [ui](){
                        Batab::getUI()->getAppSwitcher()->show();
                        Batab::getUI()->getAppSwitcher()->activateWindow();
                        Batab::getUI()->getAppSwitcher()->setFocus();
                    }, Qt::QueuedConnection);
                }

                return 1;
            }
        }
    }

    return CallNextHookEx(hookLowLevelKeyboard, nCode, wParam, lParam);
}

void CALLBACK WinProcs::winAppLifecycleEventProc(HWINEVENTHOOK hWinEventHook,
                                     DWORD event,
                                     HWND hWnd,
                                     LONG idObject,
                                     LONG idChild,
                                     DWORD idEventThread,
                                     DWORD dwmsEventTime)
{
    if (hWnd == NULL || idObject != OBJID_WINDOW || idChild != CHILDID_SELF) {
        return;
    }

    if (event == EVENT_OBJECT_DESTROY) {
        TrackedWindows::getInstance()->removeWindow(hWnd);
        return;
    }

    if (!IsWindow(hWnd)) {
        return;
    }

    if (GetAncestor(hWnd, GA_ROOT) != hWnd) {
        return;
    }

    if (!Util::isAltTabWindow(hWnd))
    {
        return;
    }

    if (event == EVENT_OBJECT_SHOW) {
        char title[512];
        GetWindowTextA(hWnd, title, sizeof(title));

        DWORD processId = 0;
        GetWindowThreadProcessId(hWnd, &processId);

        TrackedWindows::getInstance()->addWindow(hWnd,
                                                 {.title = title,
                                                  .PID = processId,
                                                  .icon = Util::getIconFromHWND(hWnd)});

        // for (const auto &[hWnd, windowDetails] :
        //      TrackedWindows::getInstance()->getWindows().asKeyValueRange()) {
        //     qDebug() << "[" << windowDetails.PID << "]: {" << hWnd << "}, " << windowDetails.title
        //              << "\n";
        // }
    }
}

void CALLBACK WinProcs::winAppNameChangeEventProc(HWINEVENTHOOK hWinEventHook,
                                                  DWORD event, HWND hWnd,
                                                  LONG idObject, LONG idChild,
                                                  DWORD idEventThread,
                                                  DWORD dwmsEventTime)
{
    if (hWnd == NULL || idObject != OBJID_WINDOW || idChild != CHILDID_SELF)
    {
        return;
    }

    if (!Util::isAltTabWindow(hWnd))
    {
        return;
    }

    const int len = GetWindowTextLengthW(hWnd);
    QString newTitle;
    if (len > 0)
    {
        QVector<wchar_t> buf(len + 1);
        GetWindowTextW(hWnd, buf.data(), len + 1);
        newTitle = QString::fromWCharArray(buf.data());
    }

    auto *tracked = TrackedWindows::getInstance();
    const auto windows = tracked->getWindows();
    const auto it = windows.constFind(hWnd);
    if (it != windows.constEnd() && it.value().title == newTitle)
    {
        return;
    }

    qDebug() << "Title actually changed:" << hWnd << newTitle;
    tracked->updateWindowTitle(hWnd, newTitle);  // see below
}

void WinProcs::registerLLKHook()
{
    if (isLLKHooked) {
        return;
    }

    hookLowLevelKeyboard = SetWindowsHookExW(WH_KEYBOARD_LL,
                                             lowLevelKeyboardProc,
                                             GetModuleHandle(NULL),
                                             0);

    if (hookLowLevelKeyboard != nullptr) {
        isLLKHooked = true;
    }
}

void WinProcs::unregisterLLKHook()
{
    if (!isLLKHooked) {
        return;
    }

    auto status = UnhookWindowsHookEx(hookLowLevelKeyboard);

    if (status) {
        hookLowLevelKeyboard = nullptr;
        isLLKHooked = false;
    }
}

void WinProcs::registerWEHooks()
{
    if (areWEHooksActive)
    {
        return;
    }

    hookWinAppLifecycleEvent =
        SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_UNCLOAKED, nullptr,
                        winAppLifecycleEventProc, 0, 0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    hookWinAppNameChangeEvent =
        SetWinEventHook(EVENT_OBJECT_NAMECHANGE, EVENT_OBJECT_NAMECHANGE,
                        nullptr, winAppNameChangeEventProc, 0, 0, NULL);

    if (hookWinAppLifecycleEvent != nullptr &&
        hookWinAppNameChangeEvent != nullptr)
    {
        areWEHooksActive = true;
    }
}

void WinProcs::unregisterWEHooks()
{
    if (!areWEHooksActive)
    {
        return;
    }

    auto statusWinAppLifecycleEvent = UnhookWinEvent(hookWinAppLifecycleEvent);
    auto statusWinAppNameChangeEvent =
        UnhookWinEvent(hookWinAppNameChangeEvent);

    if (statusWinAppLifecycleEvent && statusWinAppNameChangeEvent)
    {
        hookWinAppLifecycleEvent = nullptr;
        hookWinAppNameChangeEvent = nullptr;

        areWEHooksActive = false;
    }
}