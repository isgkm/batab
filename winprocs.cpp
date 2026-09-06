#include "winprocs.h"

#include "appswitcher.h"
#include "batab.h"
#include "trackedwindows.h"
#include "util.h"

#include <qapplication.h>
#include <QDebug>

HHOOK WinProcs::s_hookLowLevelKeyboard{nullptr};
HWINEVENTHOOK WinProcs::s_hookWinAppLifecycleEvent{nullptr};
HWINEVENTHOOK WinProcs::s_hookWinAppNameChangeEvent{nullptr};

bool WinProcs::s_isLLKHooked{};
bool WinProcs::s_areWEHooksActive{};

std::atomic<bool> WinProcs::s_switcherOpen{};

BOOL CALLBACK WinProcs::enumWindowsProc(HWND hWnd, LPARAM lparam)
{
    const int length = GetWindowTextLengthW(hWnd);
    std::wstring windowTitle;
    windowTitle.resize(length);

    GetWindowTextW(hWnd, windowTitle.data(),
                   static_cast<int>(windowTitle.size() + 1));

    if (!Util::isAltTabWindow(hWnd))
    {
        return TRUE;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(hWnd, &processId);

    TrackedWindows::getInstance()->addWindow(
        hWnd, {.title = QString::fromStdWString(windowTitle),
               .processId = processId,
               .icon = Util::getIconFromHWND(hWnd)});

    return TRUE;
}

LRESULT CALLBACK WinProcs::lowLevelKeyboardProc(int nCode, WPARAM wParam,
                                                LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        auto *keyboardHook = Util::toHandle<KBDLLHOOKSTRUCT *>(lParam);
        const bool keyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        const bool altDown = (keyboardHook->flags & LLKHF_ALTDOWN) != 0;

        if (!s_switcherOpen && keyboardHook->vkCode == VK_TAB && altDown &&
            wParam == WM_SYSKEYDOWN)
        {
            qDebug() << "alt+tab detected";
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);

            s_switcherOpen = true;

            QMetaObject::invokeMethod(
                qApp,
                []() {
                    auto *appSwitcher = Batab::getUI()->getAppSwitcher();
                    if (appSwitcher)
                    {
                        appSwitcher->show();
                        appSwitcher->activateWindow();
                        appSwitcher->setFocus();
                    }
                },
                Qt::QueuedConnection);

            return 1;
        }

        if (s_switcherOpen && keyDown && keyboardHook->vkCode == VK_TAB &&
            altDown)
        {
            const bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            QMetaObject::invokeMethod(
                Batab::getUI()->getAppSwitcher(),
                shiftDown ? "cycleSelectionBackward" : "cycleSelection",
                Qt::QueuedConnection);

            return 1;
        }
    }

    return CallNextHookEx(s_hookLowLevelKeyboard, nCode, wParam, lParam);
}

void CALLBACK WinProcs::winAppLifecycleEventProc(HWINEVENTHOOK hWinEventHook,
                                     DWORD event,
                                     HWND hWnd,
                                     LONG idObject,
                                     LONG idChild,
                                     DWORD idEventThread,
                                     DWORD dwmsEventTime)
{
    if (hWnd == nullptr || idObject != OBJID_WINDOW || idChild != CHILDID_SELF)
    {
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
        constexpr int maxTitleLength{512};
        std::vector<wchar_t> titleBuffer(maxTitleLength);
        const auto len =
            GetWindowTextW(hWnd, titleBuffer.data(), maxTitleLength);
        const QString title = QString::fromWCharArray(titleBuffer.data(), len);

        DWORD processId = 0;
        GetWindowThreadProcessId(hWnd, &processId);

        TrackedWindows::getInstance()->addWindow(
            hWnd, {.title = title,
                   .processId = processId,
                   .icon = Util::getIconFromHWND(hWnd)});
    }
}

void CALLBACK WinProcs::winAppNameChangeEventProc(HWINEVENTHOOK hWinEventHook,
                                                  DWORD event, HWND hWnd,
                                                  LONG idObject, LONG idChild,
                                                  DWORD idEventThread,
                                                  DWORD dwmsEventTime)
{
    if (hWnd == nullptr || idObject != OBJID_WINDOW || idChild != CHILDID_SELF)
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

    tracked->updateWindowTitle(hWnd, newTitle);
}

void WinProcs::registerLLKHook()
{
    if (s_isLLKHooked)
    {
        return;
    }

    s_hookLowLevelKeyboard = SetWindowsHookExW(
        WH_KEYBOARD_LL, lowLevelKeyboardProc, GetModuleHandle(nullptr), 0);

    if (s_hookLowLevelKeyboard != nullptr)
    {
        s_isLLKHooked = true;
    }
}

void WinProcs::unregisterLLKHook()
{
    if (!s_isLLKHooked)
    {
        return;
    }

    auto status = UnhookWindowsHookEx(s_hookLowLevelKeyboard);

    if (status) {
        s_hookLowLevelKeyboard = nullptr;
        s_isLLKHooked = false;
    }
}

void WinProcs::registerWEHooks()
{
    if (s_areWEHooksActive)
    {
        return;
    }

    s_hookWinAppLifecycleEvent =
        SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_UNCLOAKED, nullptr,
                        winAppLifecycleEventProc, 0, 0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    s_hookWinAppNameChangeEvent =
        SetWinEventHook(EVENT_OBJECT_NAMECHANGE, EVENT_OBJECT_NAMECHANGE,
                        nullptr, winAppNameChangeEventProc, 0, 0, 0);

    if (s_hookWinAppLifecycleEvent != nullptr &&
        s_hookWinAppNameChangeEvent != nullptr)
    {
        s_areWEHooksActive = true;
    }
}

void WinProcs::unregisterWEHooks()
{
    if (!s_areWEHooksActive)
    {
        return;
    }

    auto statusWinAppLifecycleEvent =
        UnhookWinEvent(s_hookWinAppLifecycleEvent);
    auto statusWinAppNameChangeEvent =
        UnhookWinEvent(s_hookWinAppNameChangeEvent);

    if (statusWinAppLifecycleEvent && statusWinAppNameChangeEvent)
    {
        s_hookWinAppLifecycleEvent = nullptr;
        s_hookWinAppNameChangeEvent = nullptr;

        s_areWEHooksActive = false;
    }
}