#ifndef APPSWITCHER_H
#define APPSWITCHER_H

#include <QStandardItemModel>
#include <QWidget>
#include <qplaintextedit.h>

namespace Ui {
class AppSwitcher;
}

class AppSwitcher final : public QWidget
{
    Q_OBJECT

public:
    explicit AppSwitcher(QWidget *parent = nullptr);
    ~AppSwitcher();

    void focusAppSearch();

public slots:
    void showUIAfterTimerCompleted();

private slots:
    void onTextChanged();

protected:
    void showEvent(QShowEvent *event) override;
    // void hideEvent(QHideEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::AppSwitcher *ui;
    QStandardItemModel *listModel;
    QList<HWND> *shownApps;
};

#endif // APPSWITCHER_H