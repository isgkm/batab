#ifndef UTIL_H
#define UTIL_H

#include <QIcon>
#include <QModelIndex>

class Util final
{
public:
    Util();
    [[nodiscard]] static QIcon getIconFromHWND(const HWND hWnd);
    static void focusWindowWithHWND(const HWND hWnd);
    static void focusWindowAtIndex(const QModelIndex &listIndex);
};

#endif // UTIL_H
