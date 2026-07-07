#ifndef TRACKEDWINDOWS_H
#define TRACKEDWINDOWS_H

#include <QHash>
#include <QIcon>
#include <QList>
#include <QMutex>
#include <QString>
#include <minwindef.h>

struct WindowDetails
{
    QString title;
    DWORD PID;
    QIcon icon;
};

struct WindowDetailsInternal
{
    HWND hWnd;
    QString title;
    DWORD PID;

    friend QDataStream &operator<<(QDataStream &out, const WindowDetailsInternal &idetails);
    friend QDataStream &operator>>(QDataStream &in, WindowDetailsInternal &idetails);
};

constexpr auto InternalListDataRole = Qt::UserRole + 1;

Q_DECLARE_METATYPE(WindowDetailsInternal);

class TrackedWindows final
{
public:
    TrackedWindows(TrackedWindows &other) = delete;
    void operator=(const TrackedWindows &other) = delete;

    static TrackedWindows *getInstance();

    void addWindow(HWND hWnd, const WindowDetails &windowDetails);
    bool removeWindow(const HWND hWnd);

    [[nodiscard]] inline QHash<HWND, WindowDetails> getWindows() const { return openWindows; };

protected:
    TrackedWindows() {}
    ~TrackedWindows() {}

    QHash<HWND, WindowDetails> openWindows;
    QList<HWND> openWindowsOrder;

private:
    static TrackedWindows *instance;
    static QMutex mutex;
};

#endif // TRACKEDWINDOWS_H
