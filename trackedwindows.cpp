#include "trackedwindows.h"

#include <QDebug>

TrackedWindows *TrackedWindows::instance = nullptr;
QMutex TrackedWindows::mutex;

TrackedWindows *TrackedWindows::getInstance()
{
    QMutexLocker<QMutex> lock(&mutex);

    if (instance == nullptr) {
        instance = new TrackedWindows();
    }

    return instance;
}

void TrackedWindows::addWindow(HWND hWnd, const WindowDetails &windowDetails)
{
    if (this->openWindows.contains(hWnd))
        return;

    qDebug() << "Adding: " << hWnd << " [" << windowDetails.title
             << "] to list.";

    this->openWindows.insert(hWnd, windowDetails);

    int slot{};
    if (!this->m_freeSlots.empty())
    {
        slot = m_freeSlots.firstKey();
        this->m_freeSlots.remove(slot);
        this->m_slots[slot] = hWnd;
    }
    else
    {
        slot = this->m_slots.size();
        this->m_slots.push_back(hWnd);
    }

    this->m_slotOf.insert(hWnd, slot);
    // this->openWindowOrders.emplaceBack(hWnd);
}

bool TrackedWindows::removeWindow(const HWND hWnd)
{
    if (!this->openWindows.contains(hWnd))
        return false;

    qDebug() << "Removing: " << hWnd << " from list.";

    this->openWindows.remove(hWnd);

    const int slot = this->m_slotOf.value(hWnd, -1);
    this->m_slotOf.remove(hWnd);

    if (slot >= 0)
    {
        this->m_slots[slot] = nullptr;
        this->m_freeSlots.insert(slot, true);
    }

    return true;
}

QDataStream &operator<<(QDataStream &out, const WindowDetailsInternal &idetails)
{
    out << reinterpret_cast<quintptr>(idetails.hWnd) << static_cast<quint32>(idetails.PID)
        << idetails.title;
    return out;
}

QDataStream &operator>>(QDataStream &in, WindowDetailsInternal &idetails)
{
    quintptr hwndVal;
    quint32 pidVal;

    in >> hwndVal >> pidVal >> idetails.title;

    idetails.hWnd = reinterpret_cast<HWND>(hwndVal);
    idetails.PID = static_cast<DWORD>(pidVal);

    return in;
}