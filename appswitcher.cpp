#include "appswitcher.h"

#include "indexedicondelegate.h"
#include "trackedwindows.h"
#include "ui_appswitcher.h"
#include "util.h"
#include "winprocs.h"

#include <qtimer.h>
#include <windows.h>

AppSwitcher::AppSwitcher(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AppSwitcher)
    , listModel(new QStandardItemModel())
    , selectionCommitTimer(new QTimer(this))
{
    ui->setupUi(this);

    this->ui->LV_openApps->setModel(this->listModel);
    this->ui->LV_openApps->setItemDelegate(new IndexedIconDelegate(this));

    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setFixedSize(400, 500);

    qDebug() << "appswitcher ct";

    selectionCommitTimer->setSingleShot(true);

    QObject::connect(selectionCommitTimer, &QTimer::timeout, this,
                     &AppSwitcher::activateSelectionAndHide);

    QObject::connect(qApp,
                     &QGuiApplication::applicationStateChanged,
                     this,
                     [=](Qt::ApplicationState state) {
                         if (state == Qt::ApplicationInactive) {
                             this->hide();
                             this->ui->PTE_appSearch->clear();
                         }
                     });

    QObject::connect(this->ui->LV_openApps, &QListView::clicked, &Util::focusWindowAtIndex);

    QObject::connect(this->ui->PTE_appSearch,
                     &QPlainTextEdit::textChanged,
                     this,
                     &AppSwitcher::onTextChanged);

    QObject::connect(this->ui->LV_openApps->model(),
                     &QStandardItemModel::rowsMoved,
                     this,
                     [](const QModelIndex &sourceParent,
                        int sourceStart,
                        int sourceEnd,
                        const QModelIndex &destinationParent,
                        int destinationRow) { qDebug() << "parents: " << sourceParent; });
}

AppSwitcher::~AppSwitcher()
{
    delete ui;
}

void AppSwitcher::focusAppSearch()
{
    // this->ui->PTE_appSearch->setFocus();
}

void AppSwitcher::showUIAfterTimerCompleted()
{
    this->show();
}

void AppSwitcher::onTextChanged()
{
    auto searchQuery = this->ui->PTE_appSearch->toPlainText();

    // this->ui->LV_openApps->filte
}

void AppSwitcher::showEvent(QShowEvent *event)
{
    this->listModel->clear();

    auto trackedWindows = TrackedWindows::getInstance();
    const auto ordered = trackedWindows->getOrderedWindows();
    const auto windows = trackedWindows->getWindows();

    this->listModel->setRowCount(trackedWindows->getWindowCount());
    // qDebug() << trackedWindows->getOrderedWindows();

    int row = 0;
    for (HWND hWnd : ordered)
    {
        if (!hWnd)
            continue;

        const auto it = windows.constFind(hWnd);
        if (it == windows.constEnd())
            continue;

        const WindowDetails &wDetails = it.value();

        QStandardItem *item = new QStandardItem();

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
        item->setEditable(false);
        // wDetails.title = wDetails.title;
        item->setToolTip(wDetails.title);

        auto len = wDetails.title.length();
        item->setText(len > 40 ? wDetails.title.left(40) + "..."
                               : wDetails.title);

        item->setIcon(wDetails.icon);

        WindowDetailsInternal wdi{
            .hWnd = hWnd, .title = wDetails.title, .PID = wDetails.PID};

        item->setData(QVariant::fromValue(wdi), InternalListDataRole);
        item->setData(trackedWindows->getWindowOrder(hWnd), SlotIndexRole);

        this->listModel->setItem(row, item);
        ++row;
    }

    HWND hwndForeground = GetForegroundWindow();
    DWORD foregroundThreadID =
        GetWindowThreadProcessId(hwndForeground, nullptr);
    DWORD currentThreadId = GetCurrentThreadId();

    AttachThreadInput(foregroundThreadID, currentThreadId, TRUE);
    SetForegroundWindow(reinterpret_cast<HWND>(winId()));
    SetFocus(reinterpret_cast<HWND>(winId()));
    AttachThreadInput(foregroundThreadID, currentThreadId, FALSE);

    qApp->installEventFilter(this);

    QWidget::showEvent(event);
}

void AppSwitcher::hideEvent(QHideEvent *event)
{
    selectionCommitTimer->stop();
    qApp->removeEventFilter(this);
    WinProcs::setSwitcherOpen(false);

    QWidget::hideEvent(event);
}

bool AppSwitcher::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->LV_openApps->viewport() && event->type() == QEvent::Drop)
    {
        this->handleAppReorder(static_cast<QDropEvent *>(event));
        return true;
    }

    if (event->type() == QEvent::KeyPress)
    {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();

        if (key == Qt::Key_Escape)
        {
            hide();
            return true;
        }

        if (!ui->PTE_appSearch->hasFocus() && key >= Qt::Key_1 &&
            key <= Qt::Key_9)
        {
            focusAppAtSlot(key - Qt::Key_1);
            return true;
        }

        if (!ui->PTE_appSearch->hasFocus() && key >= Qt::Key_A &&
            key <= Qt::Key_Z)
        {
            ui->PTE_appSearch->setFocus();
            QTextCursor cursor = ui->PTE_appSearch->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText(keyEvent->text());
            ui->PTE_appSearch->setTextCursor(cursor);
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void AppSwitcher::handleAppReorder(QDropEvent *event)
{
    const QModelIndex fromIndex = this->ui->LV_openApps->currentIndex();
    if (!fromIndex.isValid())
    {
        return;
    }

    const QModelIndex targetIndex =
        this->ui->LV_openApps->indexAt(event->position().toPoint());
    int toRow = targetIndex.isValid() ? targetIndex.row()
                                      : this->listModel->rowCount() - 1;
    const int fromRow = fromIndex.row();

    if (fromRow == toRow)
    {
        event->ignore();
        return;
    }

    QList<QStandardItem *> movedRow = this->listModel->takeRow(fromRow);
    if (toRow > fromRow)
        --toRow;
    this->listModel->insertRow(toRow, movedRow);

    QVector<HWND> newOrder;
    newOrder.reserve(this->listModel->rowCount());
    for (int row = 0; row < this->listModel->rowCount(); ++row)
    {
        const auto data =
            this->listModel->item(row)->data(InternalListDataRole);
        newOrder.push_back(data.value<WindowDetailsInternal>().hWnd);
    }
    TrackedWindows::getInstance()->reorderSlots(newOrder);

    for (int row = 0; row < this->listModel->rowCount(); ++row)
        this->listModel->item(row)->setData(row, SlotIndexRole);

    this->ui->LV_openApps->setCurrentIndex(listModel->index(toRow, 0));
    event->accept();
}

void AppSwitcher::focusAppAtSlot(int slot)
{
    const auto *model = ui->LV_openApps->model();

    for (int row = 0; row < model->rowCount(); ++row)
    {
        const QModelIndex index = model->index(row, 0);
        if (index.data(SlotIndexRole).toInt() != slot)
            continue;

        const QVariant data = index.data(InternalListDataRole);
        if (data.isValid() && data.canConvert<WindowDetailsInternal>())
        {
            const auto idata = data.value<WindowDetailsInternal>();

            Util::focusWindowWithHWND(idata.hWnd);
        }
        return;
    }
}

void AppSwitcher::cycleSelection()
{
    auto *model = this->ui->LV_openApps->model();

    const int rowCount = model->rowCount();

    qDebug() << "cycleSelection called, rowCount:" << rowCount
             << "currentRow:" << ui->LV_openApps->currentIndex().row()
             << "hasFocus:" << ui->LV_openApps->hasFocus();

    if (rowCount == 0)
    {
        return;
    }

    const int currentRow = this->ui->LV_openApps->currentIndex().row();
    const int nextRow = (currentRow + 1) % rowCount;

    const QModelIndex nextIndex = model->index(nextRow, 0);

    this->ui->LV_openApps->setFocus();
    this->ui->LV_openApps->setCurrentIndex(nextIndex);
    this->ui->LV_openApps->selectionModel()->select(
        nextIndex, QItemSelectionModel::ClearAndSelect);

    selectionCommitTimer->start(2000);
}

void AppSwitcher::cycleSelectionBackward()
{
    auto *model = this->ui->LV_openApps->model();

    const int rowCount = model->rowCount();
    if (rowCount == 0)
        return;

    const int currentRow = this->ui->LV_openApps->currentIndex().row();
    const int prevRow = (currentRow - 1 + rowCount) % rowCount;

    const QModelIndex prevIdx = model->index(prevRow, 0);

    this->ui->LV_openApps->setFocus();
    this->ui->LV_openApps->setCurrentIndex(prevIdx);
    this->ui->LV_openApps->selectionModel()->select(
        prevIdx, QItemSelectionModel::ClearAndSelect);

    selectionCommitTimer->start(2000);
}

void AppSwitcher::activateSelectionAndHide()
{
    selectionCommitTimer->stop();

    const QModelIndex idx = ui->LV_openApps->currentIndex();
    if (idx.isValid())
    {
        const QVariant data = idx.data(InternalListDataRole);
        if (data.isValid() && data.canConvert<WindowDetailsInternal>())
        {
            Util::focusWindowWithHWND(data.value<WindowDetailsInternal>().hWnd);
        }
    }

    this->hide();
}
