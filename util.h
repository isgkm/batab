#ifndef UTIL_H
#define UTIL_H

#include <QIcon>
#include <QModelIndex>
#include <windows.h>

#include <type_traits>

namespace Util {

[[nodiscard]] bool isAltTabWindow(HWND hWnd);
[[nodiscard]] QIcon getIconFromHWND(HWND hWnd);
[[nodiscard]] QString getFullProcessPath(DWORD processId);
[[nodiscard]] QString getAppNameFromTitle(const QString &title);

void focusWindowWithHWND(HWND hWnd);
void focusWindowAtIndex(const QModelIndex &listIndex);
void closeAppWithHWND(HWND hWnd, int slot);
void terminateAppWithHWND(HWND hWnd, int slot);

template <class T>
concept PtrLike = std::is_pointer_v<T>;

template <PtrLike From, PtrLike To>
[[nodiscard]] To reinterpretPointer(From value)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): Win32 API requires reinterpreting between pointer types (e.g. &hIcon -> PDWORD_PTR)
    return reinterpret_cast<To>(value);
}

template <typename T>
[[nodiscard]] T toHandle(ULONG_PTR value)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr)
    return reinterpret_cast<T>(value);
}

};  // namespace Util

#endif // UTIL_H
