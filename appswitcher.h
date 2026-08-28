#ifndef APPSWITCHER_H
#define APPSWITCHER_H

#include <QStandardItemModel>
#include <QWidget>

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
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    Ui::AppSwitcher *ui;
    QStandardItemModel *listModel;
    QTimer *selectionCommitTimer;

    void handleAppReorder(QDropEvent *event);
    void focusAppAtSlot(int slot);

    Q_INVOKABLE void cycleSelection();
    Q_INVOKABLE void cycleSelectionBackward();
    Q_INVOKABLE void activateSelectionAndHide();
};

#endif // APPSWITCHER_H