#ifndef WINPROCS_H
#define WINPROCS_H

#include <windows.h>

class WinProcs final
{
public:
    WinProcs() = delete;

    // get initial open windows
    static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lparam);

    // intercept alt-tab before windows handles it
    static LRESULT CALLBACK lowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    // get notified of new app starts and shutdowns
    static void CALLBACK winEventProc(HWINEVENTHOOK hWinEventHook,
                                      DWORD event,
                                      HWND hWnd,
                                      LONG idObject,
                                      LONG idChild,
                                      DWORD idEventThread,
                                      DWORD dwmsEventTime);

    static void registerLLKHook();
    static void unregisterLLKHook();
    [[nodiscard]] static inline bool getIsLLKHooked() { return isLLKHooked; }

    static void registerWEHook();
    static void unregisterWEHook();
    [[nodiscard]] static inline bool getIsWEHooked() { return isWEHooked; }

private:
    static HHOOK hookLowLevelKeyboard;
    static HWINEVENTHOOK hookWindowsEvent;

    static bool isLLKHooked;
    static bool isWEHooked;
};

#endif // WINPROCS_H