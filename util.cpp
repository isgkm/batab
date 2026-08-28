#include "util.h"

#include "trackedwindows.h"

#include <dwmapi.h>
#include <QDebug>
#include <windows.h>

bool isRootAltTabCandidate(const HWND hWnd)
{
    HWND hWndWalk = GetAncestor(hWnd, GA_ROOTOWNER);
    HWND hWndTry{};

    while ((hWndTry = GetLastActivePopup(hWndWalk)) != hWndWalk)
    {
        if (IsWindowVisible(hWndTry))
            break;

        hWndWalk = hWndTry;
    }

    return hWndWalk == hWnd;
}

bool Util::isAltTabWindow(const HWND hWnd)
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

    LONG exStyle = GetWindowLongW(hWnd, GWL_EXSTYLE);
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

QIcon Util::getIconFromHWND(const HWND hWnd)
{
    HICON hIcon = nullptr;

    auto result =
        SendMessageTimeoutW(hWnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG,
                            100, reinterpret_cast<PDWORD_PTR>(&hIcon));

    if (result == 0) {
        qDebug() << "SendMessageTimeoutW failed";
    }

    if (!hIcon) {
        hIcon = reinterpret_cast<HICON>(GetClassLongPtr(hWnd, GCLP_HICON));
    }

    if (!hIcon) {
        hIcon = LoadIconW(NULL, IDI_APPLICATION);
    }

    if (!hIcon) {
        return QIcon();
    }

    QIcon qIcon(QPixmap::fromImage(QImage::fromHICON(hIcon)));

    DestroyIcon(hIcon);

    return qIcon;
}

void Util::focusWindowWithHWND(const HWND hWnd)
{
    if (hWnd == NULL || !IsWindow(hWnd)) {
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
    auto data = listIndex.data(InternalListDataRole);
    if (data.isValid() && data.canConvert<WindowDetailsInternal>()) {
        auto idata = data.value<WindowDetailsInternal>();

        Util::focusWindowWithHWND(idata.hWnd);
    }
}