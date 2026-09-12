#ifndef TRACKEDWINDOWS_H
#define TRACKEDWINDOWS_H

#include <QDebug>
#include <QHash>
#include <QIcon>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QString>
#include <Windows.h>

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

    friend QDataStream &operator<<(QDataStream &out,
                                   const WindowDetailsInternal &idetails);
    friend QDataStream &operator>>(QDataStream &in,
                                   WindowDetailsInternal &idetails);
};

Q_DECLARE_METATYPE(WindowDetailsInternal);

class TrackedWindows final : public QObject
{
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(TrackedWindows)

    static TrackedWindows *getInstance();

    void addWindow(HWND hWnd, const WindowDetails &windowDetails);
    bool removeWindow(HWND hWnd);
    void updateWindowTitle(HWND hWnd, const QString &newTitle);
    void reorderSlots(const QVector<HWND> &newOrder);
    void queueSlotToTrack(int slot)
    {
        m_queuedSlotsToClose.insert(slot);
    }

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

signals:
    void queuedAppActuallyClosed(int slot);

protected:
    TrackedWindows() = default;
    ~TrackedWindows() override = default;

private:
    static TrackedWindows *s_instance;
    static QMutex s_mutex;

    QHash<HWND, WindowDetails> m_openWindows;

    QSet<int> m_queuedSlotsToClose;

    QVector<HWND> m_slots;
    QHash<HWND, int> m_slotOf;
    QMap<int, bool> m_freeSlots;
};

#endif // TRACKEDWINDOWS_H
