#include "winprocs.h"

#include "appswitcher.h"
#include "batab.h"
#include "settings.h"
#include "trackedwindows.h"
#include "util.h"

#include <QDebug>

#include <qapplication.h>

HHOOK WinProcs::s_hookLowLevelKeyboard{nullptr};
HWINEVENTHOOK WinProcs::s_hookWinAppLifecycleEvent{nullptr};
HWINEVENTHOOK WinProcs::s_hookWinAppNameChangeEvent{nullptr};
HWINEVENTHOOK WinProcs::s_hookForegroundAppChangeEvent{nullptr};

bool WinProcs::s_isLLKHooked{};
bool WinProcs::s_areWEHooksActive{};

std::atomic<bool> WinProcs::s_switcherOpen{};

QElapsedTimer WinProcs::s_switcherOpenTimer{};

BOOL CALLBACK WinProcs::enumWindowsProc(HWND hWnd, LPARAM lparam) {
    const int length = GetWindowTextLengthW(hWnd);
    std::wstring windowTitle;
    windowTitle.resize(length);

    GetWindowTextW(hWnd, windowTitle.data(),
                   static_cast<int>(windowTitle.size() + 1));

    if (!Util::isAltTabWindow(hWnd) || Util::isSystemWindow(hWnd)) {
        return TRUE;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(hWnd, &processId);

    TrackedWindows::getInstance().addWindow(
        hWnd, {.title = QString::fromStdWString(windowTitle),
               .processId = processId,
               .icon = Util::getIconFromHWND(hWnd)});

    return TRUE;
}

LRESULT CALLBACK WinProcs::lowLevelKeyboardProc(int nCode, WPARAM wParam,
                                                LPARAM lParam) {
    if (nCode == HC_ACTION) {
        auto* keyboardHook = Util::toHandle<KBDLLHOOKSTRUCT*>(lParam);
        const bool altDown = (keyboardHook->flags & LLKHF_ALTDOWN) != 0;
        const bool keyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        const bool keyUp = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

        if (!s_switcherOpen && keyboardHook->vkCode == VK_TAB && altDown &&
            wParam == WM_SYSKEYDOWN)
        {
            qDebug() << "[OPEN] vkCode=" << keyboardHook->vkCode
                     << "wParam=" << wParam;
            // qDebug() << "alt+tab detected";
            keybd_event(VK_CONTROL, 0, 0, 0);
            keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);

            s_switcherOpen = true;
            s_switcherOpenTimer.start();

            QMetaObject::invokeMethod(
                qApp,
                []() {
                    auto* appSwitcher = Batab::getUI()->getAppSwitcher();
                    if (appSwitcher) {
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
            qDebug() << "[CYCLE] vkCode=" << keyboardHook->vkCode
                     << "wParam=" << wParam;
            const bool shiftDown = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
            QMetaObject::invokeMethod(
                Batab::getUI()->getAppSwitcher(),
                shiftDown ? "cycleSelectionBackward" : "cycleSelection",
                Qt::QueuedConnection);

            return 1;
        }

        if (s_switcherOpen && keyUp &&
            (keyboardHook->vkCode == VK_LMENU ||
             keyboardHook->vkCode == VK_RMENU))
        {
            qDebug() << "[RELEASE] vkCode=" << keyboardHook->vkCode
                     << "wParam=" << wParam
                     << "elapsed=" << s_switcherOpenTimer.elapsed();
            s_switcherOpen = false;
            const bool wasQuickTap =
                s_switcherOpenTimer.elapsed() <
                Settings::getInstance().quickSwitchHoldThresholdMs();

            // qDebug() << "wasQucikTap: " << wasQuickTap;
            QMetaObject::invokeMethod(Batab::getUI()->getAppSwitcher(),
                                      "altReleased", Qt::QueuedConnection,
                                      Q_ARG(bool, wasQuickTap));
        }

        // Add this catch-all too, temporarily, to see EVERYTHING while switcher is open
        if (s_switcherOpen) {
            qDebug() << "[ALL] vkCode=" << keyboardHook->vkCode
                     << "wParam=" << wParam << "keyDown=" << keyDown
                     << "keyUp=" << keyUp << "altDown=" << altDown;
        }
    }

    return CallNextHookEx(s_hookLowLevelKeyboard, nCode, wParam, lParam);
}

void CALLBACK WinProcs::winAppLifecycleEventProc(HWINEVENTHOOK hWinEventHook,
                                                 DWORD event, HWND hWnd,
                                                 LONG idObject, LONG idChild,
                                                 DWORD idEventThread,
                                                 DWORD dwmsEventTime) {
    if (hWnd == nullptr || idObject != OBJID_WINDOW || idChild != CHILDID_SELF)
    {
        return;
    }

    if (event == EVENT_OBJECT_DESTROY || event == EVENT_OBJECT_HIDE ||
        event == EVENT_OBJECT_CLOAKED)
    {
        TrackedWindows::getInstance().removeWindow(hWnd);
        return;
    }

    if (!IsWindow(hWnd)) {
        return;
    }

    if (!Util::isAltTabWindow(hWnd) || Util::isSystemWindow(hWnd)) {
        return;
    }

    if (event == EVENT_OBJECT_SHOW || event == EVENT_OBJECT_UNCLOAKED) {
        constexpr int maxTitleLength{512};
        std::vector<wchar_t> titleBuffer(maxTitleLength);
        const auto len =
            GetWindowTextW(hWnd, titleBuffer.data(), maxTitleLength);
        const QString title = QString::fromWCharArray(titleBuffer.data(), len);

        if (title.isEmpty()) {
            return;
        }

        DWORD processId = 0;
        GetWindowThreadProcessId(hWnd, &processId);

        TrackedWindows::getInstance().addWindow(
            hWnd, {.title = title,
                   .processId = processId,
                   .icon = Util::getIconFromHWND(hWnd)});
    }
}

void CALLBACK WinProcs::winAppNameChangeEventProc(HWINEVENTHOOK hWinEventHook,
                                                  DWORD event, HWND hWnd,
                                                  LONG idObject, LONG idChild,
                                                  DWORD idEventThread,
                                                  DWORD dwmsEventTime) {
    if (hWnd == nullptr || idObject != OBJID_WINDOW || idChild != CHILDID_SELF)
    {
        return;
    }

    if (!Util::isAltTabWindow(hWnd) || Util::isSystemWindow(hWnd)) {
        return;
    }

    const int len = GetWindowTextLengthW(hWnd);
    QString newTitle;
    if (len > 0) {
        std::vector<wchar_t> buffer(len + 1);
        const auto gwtw = GetWindowTextW(hWnd, buffer.data(), len + 1);
        newTitle = QString::fromWCharArray(buffer.data(), gwtw);
    }

    const auto windows = TrackedWindows::getInstance().getWindows();
    const auto it = windows.constFind(hWnd);
    if (it != windows.constEnd() && it.value().title == newTitle) {
        return;
    }

    TrackedWindows::getInstance().updateWindowTitle(hWnd, newTitle);
}

void CALLBACK WinProcs::winForegroundAppChangedEventProc(
    HWINEVENTHOOK hWinEventHook, DWORD event, HWND hWnd, LONG idObject,
    LONG idChild, DWORD idEventThread, DWORD dwmsEventTime) {
    if (hWnd == nullptr || idObject != OBJID_WINDOW || idChild != CHILDID_SELF)
    {
        return;
    }

    if (TrackedWindows::getInstance().getWindows().contains(hWnd)) {
        TrackedWindows::getInstance().markWindowActivated(hWnd);
    }
}

void WinProcs::registerLLKHook() {
    if (s_isLLKHooked) {
        return;
    }

    s_hookLowLevelKeyboard = SetWindowsHookExW(
        WH_KEYBOARD_LL, lowLevelKeyboardProc, GetModuleHandle(nullptr), 0);

    if (s_hookLowLevelKeyboard != nullptr) {
        s_isLLKHooked = true;
    }
}

void WinProcs::unregisterLLKHook() {
    if (!s_isLLKHooked) {
        return;
    }

    auto status = UnhookWindowsHookEx(s_hookLowLevelKeyboard);

    if (status) {
        s_hookLowLevelKeyboard = nullptr;
        s_isLLKHooked = false;
    }
}

void WinProcs::registerWEHooks() {
    if (s_areWEHooksActive) {
        return;
    }

    s_hookWinAppLifecycleEvent =
        SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_UNCLOAKED, nullptr,
                        winAppLifecycleEventProc, 0, 0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    s_hookWinAppNameChangeEvent =
        SetWinEventHook(EVENT_OBJECT_NAMECHANGE, EVENT_OBJECT_NAMECHANGE,
                        nullptr, winAppNameChangeEventProc, 0, 0,
                        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    s_hookForegroundAppChangeEvent = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr,
        winForegroundAppChangedEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);

    if (s_hookWinAppLifecycleEvent != nullptr &&
        s_hookWinAppNameChangeEvent != nullptr &&
        s_hookForegroundAppChangeEvent != nullptr)
    {
        s_areWEHooksActive = true;
    }
}

void WinProcs::unregisterWEHooks() {
    if (!s_areWEHooksActive) {
        return;
    }

    auto statusWinAppLifecycleEvent =
        UnhookWinEvent(s_hookWinAppLifecycleEvent);
    auto statusWinAppNameChangeEvent =
        UnhookWinEvent(s_hookWinAppNameChangeEvent);
    auto statusWinForegroundAppChangeEvent =
        UnhookWinEvent(s_hookForegroundAppChangeEvent);

    if (statusWinAppLifecycleEvent && statusWinAppNameChangeEvent &&
        statusWinForegroundAppChangeEvent)
    {
        s_hookWinAppLifecycleEvent = nullptr;
        s_hookWinAppNameChangeEvent = nullptr;
        s_hookForegroundAppChangeEvent = nullptr;

        s_areWEHooksActive = false;
    }
}
