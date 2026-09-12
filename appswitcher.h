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
    void removeQueuedAppToClose(int slot)
    {
        m_listModel->removeRow(slot);
    }

private slots:
    void onTextChanged();
    void customContextMenuRequested(const QPoint &pos);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    Ui::AppSwitcher *m_ui;
    QStandardItemModel *m_listModel;
    QTimer *m_selectionCommitTimer;
    IndexedIconDelegate *m_iconDelegate;

    QMenu *m_customItemContextMenu;
    QAction *m_actionCloseApp;
    QAction *m_actionTerminateApp;
    QAction *m_actionAddAppRuleByName;
    QAction *m_actionAddAppRuleByPath;

    bool m_timerShouldStart{};
    bool m_hasNavigated{};

    void handleAppReorder(QDropEvent *event);
    void focusAppAtSlot(int slot);

    Q_INVOKABLE void cycleSelection();
    Q_INVOKABLE void cycleSelectionBackward();
    Q_INVOKABLE void activateSelectionAndHide();
};

#endif // APPSWITCHER_H