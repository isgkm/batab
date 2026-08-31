#ifndef WINPROCS_H
#define WINPROCS_H

#include <windows.h>

#include <atomic>

class WinProcs final
{
public:
    WinProcs() = delete;

    // get initial open windows
    static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lparam);

    // intercept alt-tab before windows handles it
    static LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    // get notified of new app starts and shutdowns
    static void CALLBACK winAppLifecycleEventProc(HWINEVENTHOOK hWinEventHook,
                                                  DWORD event, HWND hWnd,
                                                  LONG idObject, LONG idChild,
                                                  DWORD idEventThread,
                                                  DWORD dwmsEventTime);

    static void CALLBACK winAppNameChangeEventProc(HWINEVENTHOOK hWinEventHook,
                                                   DWORD event, HWND hWnd,
                                                   LONG idObject, LONG idChild,
                                                   DWORD idEventThread,
                                                   DWORD dwmsEventTime);

    static void setSwitcherOpen(bool open)
    {
        s_switcherOpen = open;
    }

    static void registerLLKHook();
    static void unregisterLLKHook();
    [[nodiscard]] static bool getIsLLKHooked()
    {
        return s_isLLKHooked;
    }

    static void registerWEHooks();
    static void unregisterWEHooks();
    [[nodiscard]] static bool getIsWEHooked()
    {
        return s_areWEHooksActive;
    }

private:
    static HHOOK s_hookLowLevelKeyboard;
    static HWINEVENTHOOK s_hookWinAppLifecycleEvent;
    static HWINEVENTHOOK s_hookWinAppNameChangeEvent;

    static bool s_isLLKHooked;
    static bool s_areWEHooksActive;
    static std::atomic<bool> s_switcherOpen;
};

#endif // WINPROCS_H