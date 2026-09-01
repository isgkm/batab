#include "util.h"

#include "constants.h"
#include "trackedwindows.h"

#include <dwmapi.h>
#include <QDebug>
#include <windows.h>

namespace {
bool isRootAltTabCandidate(HWND hWnd)
{
    HWND hWndWalk = GetAncestor(hWnd, GA_ROOTOWNER);
    HWND hWndTry{};

    while ((hWndTry = GetLastActivePopup(hWndWalk)) != hWndWalk)
    {
        if (IsWindowVisible(hWndTry))
        {
            break;
        }

        hWndWalk = hWndTry;
    }

    return hWndWalk == hWnd;
}
}  // namespace

bool Util::isAltTabWindow(HWND hWnd)
{
    if (GetWindowTextLengthW(hWnd) == 0)
    {
        return false;
    }

    if (!IsWindowVisible(hWnd))
    {
        return false;
    }

    if (!isRootAltTabCandidate(hWnd))
    {
        return false;
    }

    const LONG exStyle = GetWindowLongW(hWnd, GWL_EXSTYLE);
    if ((exStyle & WS_EX_TOOLWINDOW) && !(exStyle & WS_EX_APPWINDOW))
    {
        return false;
    }

    DWORD cloaked = FALSE;
    const HRESULT result =
        DwmGetWindowAttribute(hWnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));

    if (result == S_OK && cloaked)
    {
        return false;
    }

    return true;
}

QIcon Util::getIconFromHWND(HWND hWnd)
{
    HICON hIcon{};

    auto result = SendMessageTimeoutW(
        hWnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG, 100,
        Util::reinterpretPointer<HICON, PDWORD_PTR>(
            hIcon));  // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast): Win32 API requirement

    if (result == 0) {
        qDebug() << "SendMessageTimeoutW failed";
    }

    if (!hIcon) {
        hIcon = Util::toHandle<HICON>(GetClassLongPtr(hWnd, GCLP_HICON));
    }

    if (!hIcon) {
        hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    }

    if (!hIcon) {
        return {};
    }

    HICON hIconOwned = CopyIcon(hIcon);
    if (!hIconOwned)
    {
        return {};
    }

    QIcon qIcon(QPixmap::fromImage(QImage::fromHICON(hIconOwned)));

    DestroyIcon(hIconOwned);

    return qIcon;
}

void Util::focusWindowWithHWND(HWND hWnd)
{
    if (hWnd == nullptr || !IsWindow(hWnd))
    {
        return;
    }

    if (IsIconic(hWnd)) {
        ShowWindow(hWnd, SW_RESTORE);
    } else {
        ShowWindow(hWnd, SW_SHOW);
    }

    SetForegroundWindow(hWnd);
}

void Util::focusWindowAtIndex(const QModelIndex &listIndex)
{
    auto data = listIndex.data(Constants::INTERNAL_LIST_DATA_ROLE);
    if (data.isValid() && data.canConvert<WindowDetailsInternal>()) {
        auto idata = data.value<WindowDetailsInternal>();

        Util::focusWindowWithHWND(idata.hWnd);
    }
}