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

    static void setSwitcherOpen(bool open);

    static void registerLLKHook();
    static void unregisterLLKHook();
    [[nodiscard]] static inline bool getIsLLKHooked() { return isLLKHooked; }

    static void registerWEHooks();
    static void unregisterWEHooks();
    [[nodiscard]] static inline bool getIsWEHooked()
    {
        return areWEHooksActive;
    }

private:
    static HHOOK hookLowLevelKeyboard;
    static HWINEVENTHOOK hookWinAppLifecycleEvent;
    static HWINEVENTHOOK hookWinAppNameChangeEvent;

    static bool isLLKHooked;
    static bool areWEHooksActive;
    static std::atomic<bool> g_switcherOpen;
};

#endif // WINPROCS_H