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
        Util::reinterpretPointer<HICON, PDWORD_PTR>(hIcon));

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
    auto data = listIndex.data(Constants::ROLE_INTERNAL_LIST_DATA);
    if (data.isValid() && data.canConvert<WindowDetailsInternal>()) {
        auto idata = data.value<WindowDetailsInternal>();

        Util::focusWindowWithHWND(idata.hWnd);
    }
}

QString Util::getFullProcessPath(DWORD processId)
{
    HANDLE handle =
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);

    if (!handle)
    {
        return {};
    }

    auto cleanup = qScopeGuard([handle] {
        CloseHandle(handle);
    });

    DWORD bufferSize = 512;
    std::wstring buffer(bufferSize, L'\0');

    if (QueryFullProcessImageNameW(handle, 0, buffer.data(), &bufferSize) == 0)
    {
        return {};
    }

    return QString::fromWCharArray(buffer.data(),
                                   static_cast<qsizetype>(bufferSize));
}

QString Util::getAppNameFromTitle(const QString &appTitle)
{
    static const QVector<QString> s_separators{
        QStringLiteral(" - "), QStringLiteral(" – "), QStringLiteral(" — ")};

    qsizetype bestIndex{-1};
    QStringView matchedSep{};
    for (const auto &sep : s_separators)
    {
        const auto idx = appTitle.lastIndexOf(sep);
        if (idx > bestIndex)
        {
            bestIndex = idx;
            matchedSep = sep;
        }
    }

    if (bestIndex < 0)
    {
        return appTitle;
    }

    const QString candidate =
        appTitle.mid(bestIndex + matchedSep.length()).trimmed();

    return candidate.isEmpty() ? appTitle : candidate;
}

void Util::closeAppWithHWND(HWND hWnd, int slot)
{
    if (hWnd == nullptr)
    {
        return;
    }

    if (slot >= 0)
    {
        TrackedWindows::getInstance()->queueSlotToTrack(slot);
    }

    PostMessageW(hWnd, WM_CLOSE, 0, 0);
}

void Util::terminateAppWithHWND(HWND hWnd, int slot)
{
    if (hWnd == nullptr)
    {
        return;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(hWnd, &processId);
    if (processId == 0)
    {
        return;
    }

    HANDLE handle = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (!handle)
    {
        return;
    }

    auto cleanup = qScopeGuard([handle] {
        CloseHandle(handle);
    });

    if (slot >= 0)
    {
        TrackedWindows::getInstance()->queueSlotToTrack(slot);
    }

    TerminateProcess(handle, 1);
}
