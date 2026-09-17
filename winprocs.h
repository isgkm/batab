#ifndef WINPROCS_H
#define WINPROCS_H

#include <QElapsedTimer>
#include <QtClassHelperMacros>

#include <atomic>
#include <windows.h>

class WinProcs final {
  public:
    WinProcs() = default;
    ~WinProcs() = default;

    Q_DISABLE_COPY_MOVE(WinProcs)

    // get initial open windows
    static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lparam);

    // intercept alt-tab before windows handles it
    static LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam,
                                                 LPARAM lParam);

    // get notified of new app starts and shutdowns
    static void CALLBACK winAppLifecycleEventProc(HWINEVENTHOOK hWinEventHook,
                                                  DWORD event, HWND hWnd,
                                                  LONG idObject, LONG idChild,
                                                  DWORD idEventThread,
                                                  DWORD dwmsEventTime);

    // get notified of app title changes
    static void CALLBACK winAppNameChangeEventProc(HWINEVENTHOOK hWinEventHook,
                                                   DWORD event, HWND hWnd,
                                                   LONG idObject, LONG idChild,
                                                   DWORD idEventThread,
                                                   DWORD dwmsEventTime);

    static void CALLBACK winForegroundAppChangedEventProc(
        HWINEVENTHOOK hWinEventHook, DWORD event, HWND hWnd, LONG idObject,
        LONG idChild, DWORD idEventThread, DWORD dwmsEventTime);

    static void setSwitcherOpen(bool open) { s_switcherOpen = open; }

    static void registerLLKHook();
    static void unregisterLLKHook();
    [[nodiscard]] static bool getIsLLKHooked() { return s_isLLKHooked; }

    static void registerWEHooks();
    static void unregisterWEHooks();
    [[nodiscard]] static bool getIsWEHooked() { return s_areWEHooksActive; }

  private:
    static HHOOK s_hookLowLevelKeyboard;
    static HWINEVENTHOOK s_hookWinAppLifecycleEvent;
    static HWINEVENTHOOK s_hookWinAppNameChangeEvent;
    static HWINEVENTHOOK s_hookForegroundAppChangeEvent;

    static bool s_isLLKHooked;
    static bool s_areWEHooksActive;

    static std::atomic<bool> s_switcherOpen;

    static QElapsedTimer s_switcherOpenTimer;
};

#endif  // WINPROCS_H
