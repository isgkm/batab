#include "util.h"
#include <QDebug>
#include "trackedwindows.h"
#include <windows.h>

Util::Util() {}

QIcon Util::getIconFromHWND(const HWND hWnd)
{
    HICON hIcon = nullptr;

    auto result = SendMessageTimeoutW(hWnd,
                                      WM_GETICON,
                                      ICON_BIG,
                                      NULL,
                                      SMTO_ABORTIFHUNG,
                                      100,
                                      reinterpret_cast<PDWORD_PTR>(&hIcon));

    if (result == 0) {
        qDebug() << "SendMessageTimeoutW failed";
    }

    if (hIcon == nullptr) {
        qDebug() << "hIcon is nullptr";
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