#ifndef TRACKEDWINDOWS_H
#define TRACKEDWINDOWS_H

#include <minwindef.h>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>

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
    [[nodiscard]] inline int getWindowOrder(HWND hWnd) const
    {
        return m_slotOf.value(hWnd, -1);
    }

    [[nodiscard]] inline QVector<HWND> getOrderedWindows() const
    {
        return m_slots;
    }

    [[nodiscard]] inline auto getWindowCount() const
    {
        return openWindows.size();
    }

protected:
    TrackedWindows() {}
    ~TrackedWindows() {}

    QHash<HWND, WindowDetails> openWindows;

    QVector<HWND> m_slots;
    QHash<HWND, int> m_slotOf;
    QMap<int, bool> m_freeSlots;

private:
    static TrackedWindows *instance;
    static QMutex mutex;

    int windowOrderIdx{0};
};

#endif // TRACKEDWINDOWS_H
