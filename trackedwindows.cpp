#include "trackedwindows.h"

#include "util.h"

#include <QDebug>

TrackedWindows *TrackedWindows::s_instance{};
QMutex TrackedWindows::s_mutex{};

QDataStream &operator<<(QDataStream &out, const WindowDetailsInternal &idetails)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast, performance-no-int-to-ptr) - HWND is a pointer typedef; storing as a plain integer for serialization
    out << reinterpret_cast<quintptr>(idetails.hWnd) << idetails.title
        << static_cast<quint32>(idetails.processId);
    return out;
}

QDataStream &operator>>(QDataStream &in, WindowDetailsInternal &idetails)
{
    quintptr hwndValue{};
    quint32 processId{};
    in >> hwndValue >> idetails.title >> processId;
    idetails.hWnd = Util::toHandle<HWND>(hwndValue);
    idetails.processId = processId;
    return in;
}

TrackedWindows *TrackedWindows::getInstance()
{
    const QMutexLocker<QMutex> lock(&s_mutex);

    if (s_instance == nullptr)
    {
        s_instance = new TrackedWindows();
    }

    return s_instance;
}

void TrackedWindows::addWindow(HWND hWnd, const WindowDetails &windowDetails)
{
    if (m_openWindows.contains(hWnd))
    {
        return;
    }

    qDebug() << "Adding: " << hWnd << " [" << windowDetails.title
             << "] to list.";

    m_openWindows.insert(hWnd, windowDetails);

    int slot{};
    if (!m_freeSlots.empty())
    {
        slot = m_freeSlots.firstKey();
        m_freeSlots.remove(slot);
        // TODO CHECK: m_slots[slot] = hWnd;
        m_slots.replace(slot, hWnd);
    }
    else
    {
        slot = static_cast<int>(m_slots.size());
        m_slots.push_back(hWnd);
    }

    m_slotOf.insert(hWnd, slot);
    // openWindowOrders.emplaceBack(hWnd);
}

bool TrackedWindows::removeWindow(HWND hWnd)
{
    if (!m_openWindows.contains(hWnd))
    {
        return false;
    }

    qDebug() << "Removing: " << hWnd << " from list.";

    m_openWindows.remove(hWnd);

    const int slot = m_slotOf.value(hWnd, -1);
    m_slotOf.remove(hWnd);

    if (slot >= 0)
    {
        m_slots.replace(slot, nullptr);
        m_freeSlots.insert(slot, true);
    }

    return true;
}

void TrackedWindows::updateWindowTitle(HWND hWnd, const QString &newTitle)
{
    auto it = m_openWindows.find(hWnd);
    if (it == m_openWindows.end())
    {
        return;
    }

    it->title = newTitle;
}

void TrackedWindows::reorderSlots(const QVector<HWND> &newOrder)
{
    m_slots = newOrder;
    m_slotOf.clear();
    m_freeSlots.clear();
    for (int i = 0; i < m_slots.size(); ++i)
    {
        auto *val = m_slots.value(i);
        if (val)
        {
            m_slotOf.insert(val, i);
        }
    }
}

// QDataStream &operator<<(QDataStream &out, const WindowDetailsInternal &idetails)
// {
//     out << reinterpret_cast<quintptr>(idetails.hWnd) << static_cast<quint32>(idetails.PID)
//         << idetails.title;
//     return out;
// }

// QDataStream &operator>>(QDataStream &in, WindowDetailsInternal &idetails)
// {
//     quintptr hwndVal;
//     quint32 pidVal;

//     in >> hwndVal >> pidVal >> idetails.title;

//     idetails.hWnd = reinterpret_cast<HWND>(hwndVal);
//     idetails.PID = static_cast<DWORD>(pidVal);

//     return in;
// }
