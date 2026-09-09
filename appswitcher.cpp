#include "appswitcher.h"

#include "constants.h"
#include "indexedicondelegate.h"
#include "trackedwindows.h"
#include "ui_appswitcher.h"
#include "util.h"
#include "winprocs.h"

#include <qtimer.h>
#include <windows.h>

AppSwitcher::AppSwitcher(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::AppSwitcher)
    , m_listModel(new QStandardItemModel(this))
    , m_selectionCommitTimer(new QTimer(this))
    , m_iconDelegate(new IndexedIconDelegate(this))
{
    m_ui->setupUi(this);

    m_ui->LV_openApps->setModel(m_listModel);
    m_ui->LV_openApps->setItemDelegate(m_iconDelegate);
    m_ui->LV_openApps->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ui->LV_openApps->setDragDropMode(QAbstractItemView::DragDrop);
    m_ui->LV_openApps->setDefaultDropAction(Qt::MoveAction);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);
    constexpr int width = 400;
    constexpr int height = 500;
    setFixedSize(width, height);

    m_selectionCommitTimer->setSingleShot(true);

    QObject::connect(m_selectionCommitTimer, &QTimer::timeout, this,
                     &AppSwitcher::activateSelectionAndHide);

    QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this,
                     [this](Qt::ApplicationState state) {
                         if (state == Qt::ApplicationInactive && isVisible())
                         {
                             hide();
                         }
                     });

    QObject::connect(m_ui->LV_openApps->selectionModel(),
                     &QItemSelectionModel::currentChanged, this, [this]() {
                         m_hasNavigated = true;
                         if (m_timerShouldStart)
                         {
                             m_selectionCommitTimer->start(
                                 Constants::SELECTION_COMMIT_TIMEOUT_MS);
                         }
                     });

    QObject::connect(m_ui->LV_openApps, &QListView::clicked,
                     &Util::focusWindowAtIndex);

    QObject::connect(m_ui->PTE_appSearch, &QPlainTextEdit::textChanged, this,
                     &AppSwitcher::onTextChanged);
}

AppSwitcher::~AppSwitcher()
{
    delete m_ui;
}

void AppSwitcher::showUIAfterTimerCompleted()
{
    show();
}

void AppSwitcher::onTextChanged()
{
    const auto searchQuery = m_ui->PTE_appSearch->toPlainText().trimmed();

    for (int row = 0; row < m_listModel->rowCount(); ++row)
    {
        const QStandardItem *item = m_listModel->item(row);
        if (!item)
        {
            continue;
        }

        const bool matches =
            searchQuery.isEmpty() ||
            item->text().contains(searchQuery, Qt::CaseInsensitive);

        m_ui->LV_openApps->setRowHidden(row, !matches);
    }

    int matchCount{0};
    int lastMatchingRow{-1};
    for (int row = 0; row < m_listModel->rowCount(); ++row)
    {
        if (!m_ui->LV_openApps->isRowHidden(row))
        {
            ++matchCount;
            lastMatchingRow = row;
        }
    }

    if (matchCount == 1)
    {
        const QModelIndex onlyMatch = m_listModel->index(lastMatchingRow, 0);
        m_ui->LV_openApps->setCurrentIndex(onlyMatch);
        m_hasNavigated = true;
    }
}

void AppSwitcher::showEvent(QShowEvent *event)
{
    m_listModel->clear();

    auto *trackedWindows = TrackedWindows::getInstance();
    const auto ordered = trackedWindows->getOrderedWindows();
    const auto windows = trackedWindows->getWindows();

    m_listModel->setRowCount(static_cast<int>(ordered.size()));

    int row{0};
    for (HWND hWnd : ordered)
    {
        if (!hWnd)
        {
            continue;
        }

        const auto it = windows.constFind(hWnd);
        if (it == windows.constEnd())
        {
            continue;
        }

        const WindowDetails &wDetails = it.value();

        auto *item = new QStandardItem();

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                       Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
        item->setEditable(false);
        item->setToolTip(wDetails.title);
        item->setText(wDetails.title);
        item->setIcon(wDetails.icon);

        const WindowDetailsInternal wdi{.hWnd = hWnd,
                                        .title = wDetails.title,
                                        .processId = wDetails.processId};

        item->setData(QVariant::fromValue(wdi),
                      Constants::INTERNAL_LIST_DATA_ROLE);
        item->setData(trackedWindows->getWindowOrder(hWnd),
                      Constants::SLOT_INDEX_ROLE);

        m_listModel->setItem(row, item);
        ++row;
    }

    m_listModel->setRowCount(row);

    m_ui->LV_openApps->setFocus();
    m_hasNavigated = false;
    m_timerShouldStart = false;
    if (m_selectionCommitTimer->isActive())
    {
        m_selectionCommitTimer->stop();
    }

    qApp->installEventFilter(this);

    QWidget::showEvent(event);
}

void AppSwitcher::hideEvent(QHideEvent *event)
{
    m_selectionCommitTimer->stop();
    qApp->removeEventFilter(this);
    WinProcs::setSwitcherOpen(false);

    if (!m_ui->PTE_appSearch->toPlainText().isEmpty())
    {
        m_ui->PTE_appSearch->clear();
    }

    QWidget::hideEvent(event);
}

bool AppSwitcher::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_ui->LV_openApps->viewport() && event->type() == QEvent::Drop)
    {
        handleAppReorder(dynamic_cast<QDropEvent *>(event));
        return true;
    }

    if (event->type() == QEvent::KeyPress)
    {
        auto *keyEvent = dynamic_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();

        if (key == Qt::Key_Tab || key == Qt::Key_Backtab ||
            key == Qt::Key_Down || key == Qt::Key_Up)
        {
            if (key == Qt::Key_Backtab || key == Qt::Key_Up ||
                (keyEvent->modifiers() & Qt::ShiftModifier))
            {
                m_timerShouldStart = true;
                cycleSelectionBackward();
            }
            else
            {
                m_timerShouldStart = true;
                cycleSelection();
            }
            return true;
        }

        if (key == Qt::Key_Return || key == Qt::Key_Enter)
        {
            activateSelectionAndHide();
            return true;
        }

        if (key == Qt::Key_Escape)
        {
            hide();
            return true;
        }

        if (!m_ui->PTE_appSearch->hasFocus() && key >= Qt::Key_1 &&
            key <= Qt::Key_9)
        {
            focusAppAtSlot(key - Qt::Key_1);
            return true;
        }

        if (!m_ui->PTE_appSearch->hasFocus() && key >= Qt::Key_A &&
            key <= Qt::Key_Z)
        {
            m_ui->PTE_appSearch->setFocus();
            QTextCursor cursor = m_ui->PTE_appSearch->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText(keyEvent->text());
            m_ui->PTE_appSearch->setTextCursor(cursor);
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void AppSwitcher::handleAppReorder(QDropEvent *event)
{
    event->setDropAction(Qt::CopyAction);
    const QModelIndex fromIndex = m_ui->LV_openApps->currentIndex();
    if (!fromIndex.isValid())
    {
        return;
    }

    const QModelIndex targetIndex =
        m_ui->LV_openApps->indexAt(event->position().toPoint());
    int toRow =
        targetIndex.isValid() ? targetIndex.row() : m_listModel->rowCount() - 1;
    const int fromRow = fromIndex.row();

    if (targetIndex.isValid())
    {
        const QRect targetRect = m_ui->LV_openApps->visualRect(targetIndex);
        const bool droppedInLowerHalf =
            event->position().y() > targetRect.center().y();
        if (droppedInLowerHalf)
        {
            ++toRow;
        }
    }

    if (fromRow == toRow)
    {
        event->ignore();
        return;
    }

    const QList<QStandardItem *> movedRow = m_listModel->takeRow(fromRow);
    if (toRow > fromRow)
    {
        --toRow;
    }
    m_listModel->insertRow(toRow, movedRow);

    QVector<HWND> newOrder;
    newOrder.reserve(m_listModel->rowCount());
    for (int row = 0; row < m_listModel->rowCount(); ++row)
    {
        const auto data =
            m_listModel->item(row)->data(Constants::INTERNAL_LIST_DATA_ROLE);
        newOrder.push_back(data.value<WindowDetailsInternal>().hWnd);
    }
    TrackedWindows::getInstance()->reorderSlots(newOrder);

    for (int row = 0; row < m_listModel->rowCount(); ++row)
    {
        m_listModel->item(row)->setData(row, Constants::SLOT_INDEX_ROLE);
    }

    m_ui->LV_openApps->setCurrentIndex(m_listModel->index(toRow, 0));
    m_timerShouldStart = false;

    event->accept();
}

void AppSwitcher::focusAppAtSlot(int slot)
{
    const auto *model = m_ui->LV_openApps->model();

    for (int row = 0; row < model->rowCount(); ++row)
    {
        const QModelIndex index = model->index(row, 0);
        if (index.data(Constants::SLOT_INDEX_ROLE).toInt() != slot)
        {
            continue;
        }

        const QVariant data = index.data(Constants::INTERNAL_LIST_DATA_ROLE);
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
    auto *model = m_ui->LV_openApps->model();
    const int rowCount = model->rowCount();
    if (rowCount == 0)
    {
        return;
    }

    int nextRow{};
    bool needsAdvance{true};

    if (!m_hasNavigated)
    {
        const int currentRow = m_ui->LV_openApps->currentIndex().row();
        if (currentRow >= 0 && !m_ui->LV_openApps->isRowHidden(currentRow))
        {
            nextRow = currentRow;
            needsAdvance = false;
        }
        else
        {
            nextRow = -1;
        }
    }
    else
    {
        nextRow = m_ui->LV_openApps->currentIndex().row();
    }

    if (needsAdvance)
    {
        bool found = false;
        for (int i = 0; i < rowCount; ++i)
        {
            nextRow = (nextRow + 1) % rowCount;
            if (!m_ui->LV_openApps->isRowHidden(nextRow))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return;
        }
    }

    const QModelIndex nextIndex = model->index(nextRow, 0);
    m_ui->LV_openApps->setFocus();
    m_ui->LV_openApps->setCurrentIndex(nextIndex);
    m_ui->LV_openApps->selectionModel()->select(
        nextIndex, QItemSelectionModel::ClearAndSelect);
    m_ui->LV_openApps->viewport()->update();
    m_hasNavigated = true;
}

void AppSwitcher::cycleSelectionBackward()
{
    auto *model = m_ui->LV_openApps->model();
    const int rowCount = model->rowCount();
    if (rowCount == 0)
    {
        return;
    }

    int prevRow{};
    bool needsAdvance{true};

    if (!m_hasNavigated)
    {
        const int currentRow = m_ui->LV_openApps->currentIndex().row();
        if (currentRow >= 0 && !m_ui->LV_openApps->isRowHidden(currentRow))
        {
            prevRow = currentRow;
            needsAdvance = false;
        }
        else
        {
            prevRow = 0;
        }
    }
    else
    {
        prevRow = m_ui->LV_openApps->currentIndex().row();
    }

    if (needsAdvance)
    {
        bool found{};
        for (int i = 0; i < rowCount; ++i)
        {
            prevRow = (prevRow - 1 + rowCount) % rowCount;
            if (!m_ui->LV_openApps->isRowHidden(prevRow))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            return;
        }
    }

    const QModelIndex prevIndex = model->index(prevRow, 0);
    m_ui->LV_openApps->setFocus();
    m_ui->LV_openApps->setCurrentIndex(prevIndex);
    m_ui->LV_openApps->selectionModel()->select(
        prevIndex, QItemSelectionModel::ClearAndSelect);
    m_ui->LV_openApps->viewport()->update();
    m_hasNavigated = true;
}

void AppSwitcher::activateSelectionAndHide()
{
    m_selectionCommitTimer->stop();

    const QModelIndex idx = m_ui->LV_openApps->currentIndex();
    if (idx.isValid())
    {
        const QVariant data = idx.data(Constants::INTERNAL_LIST_DATA_ROLE);
        if (data.isValid() && data.canConvert<WindowDetailsInternal>())
        {
            Util::focusWindowWithHWND(data.value<WindowDetailsInternal>().hWnd);
        }
    }

    hide();
}
