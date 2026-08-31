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
    DWORD processId;
    QIcon icon;
};

struct WindowDetailsInternal
{
    HWND hWnd{};
    QString title{};
    DWORD processId{};

    // friend QDataStream &operator<<(QDataStream &out, const WindowDetailsInternal &idetails);
    // friend QDataStream &operator>>(QDataStream &in, WindowDetailsInternal &idetails);
};

Q_DECLARE_METATYPE(WindowDetailsInternal);

class TrackedWindows final
{
public:
    TrackedWindows() = default;
    ~TrackedWindows() = default;

    Q_DISABLE_COPY_MOVE(TrackedWindows)

    static TrackedWindows *getInstance();

    void addWindow(HWND hWnd, const WindowDetails &windowDetails);
    bool removeWindow(HWND hWnd);
    void updateWindowTitle(HWND hWnd, const QString &newTitle);
    void reorderSlots(const QVector<HWND> &newOrder);

    [[nodiscard]] QHash<HWND, WindowDetails> getWindows() const
    {
        return m_openWindows;
    };

    [[nodiscard]] int getWindowOrder(HWND hWnd) const
    {
        return m_slotOf.value(hWnd, -1);
    }

    [[nodiscard]] QVector<HWND> getOrderedWindows() const
    {
        return m_slots;
    }

    [[nodiscard]] auto getWindowCount() const
    {
        return m_openWindows.size();
    }

private:
    static TrackedWindows *s_instance;
    static QMutex s_mutex;

    QHash<HWND, WindowDetails> m_openWindows;

    QVector<HWND> m_slots;
    QHash<HWND, int> m_slotOf;
    QMap<int, bool> m_freeSlots;
};

#endif // TRACKEDWINDOWS_H
