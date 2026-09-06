#ifndef APPSWITCHER_H
#define APPSWITCHER_H

#include "indexedicondelegate.h"

#include <QStandardItemModel>
#include <QWidget>

namespace Ui {
class AppSwitcher;
}  // namespace Ui

class AppSwitcher final : public QWidget
{
    Q_OBJECT

public:
    explicit AppSwitcher(QWidget *parent = nullptr);
    ~AppSwitcher() override;

    Q_DISABLE_COPY_MOVE(AppSwitcher)

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void showUIAfterTimerCompleted();

private slots:
    void onTextChanged();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    Ui::AppSwitcher *m_ui;
    QStandardItemModel *m_listModel;
    QTimer *m_selectionCommitTimer;
    IndexedIconDelegate *m_iconDelegate;
    bool m_hasNavigated{};

    void handleAppReorder(QDropEvent *event);
    void focusAppAtSlot(int slot);

    Q_INVOKABLE void cycleSelection();
    Q_INVOKABLE void cycleSelectionBackward();
    Q_INVOKABLE void activateSelectionAndHide();
};

#endif // APPSWITCHER_H