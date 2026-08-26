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
    if (!this->openWindows.contains(hWnd)) {
        qDebug() << "Adding: " << hWnd << " [" << windowDetails.title << "] to list.";

        this->openWindows.emplace(hWnd, windowDetails);

        openWindowOrders.append(hWnd);
    }
}

bool TrackedWindows::removeWindow(const HWND hWnd)
{
    if (this->openWindows.contains(hWnd)) {
        qDebug() << "Removing: " << hWnd << " from list.";

        openWindowOrders.removeFirst();

        return this->openWindows.remove(hWnd);
    }

    return false;
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