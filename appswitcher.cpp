#include "appswitcher.h"

#include "constants.h"
#include "indexedicondelegate.h"
#include "settings.h"
#include "trackedwindows.h"
#include "ui_appswitcher.h"
#include "util.h"
#include "winprocs.h"

#include <QMenu>
#include <QTimer>

#include <windows.h>

AppSwitcher::AppSwitcher(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::AppSwitcher),
      m_listModel(new QStandardItemModel(this)),
      m_selectionCommitTimer(new QTimer(this)),
      m_iconDelegate(new IndexedIconDelegate(this)),
      m_slotInputTimer(new QTimer(this)),
      m_customItemContextMenu(new QMenu(this)) {
    m_ui->setupUi(this);

    m_ui->LV_openApps->setContextMenuPolicy(Qt::CustomContextMenu);

    m_ui->LV_openApps->setModel(m_listModel);
    m_ui->LV_openApps->setItemDelegate(m_iconDelegate);
    m_ui->LV_openApps->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_ui->LV_openApps->setDragDropMode(QAbstractItemView::DragDrop);
    m_ui->LV_openApps->setDefaultDropAction(Qt::MoveAction);

    m_actionAddAppRuleByName =
        m_customItemContextMenu->addAction(tr("Add name rule for %1"));
    m_actionAddAppRuleByPath =
        m_customItemContextMenu->addAction(tr("Add path rule for %1"));
    m_actionTerminateApp =
        m_customItemContextMenu->addAction(tr("Terminate %1"));
    m_actionCloseApp = m_customItemContextMenu->addAction(tr("Close %1"));

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint);
    constexpr int width = 400;
    constexpr int height = 500;
    setFixedSize(width, height);

    m_selectionCommitTimer->setSingleShot(true);

    connect(m_selectionCommitTimer, &QTimer::timeout, this,
            &AppSwitcher::activateSelectionAndHide);

    m_slotInputTimer->setSingleShot(true);
    connect(m_slotInputTimer, &QTimer::timeout, this, [this]() {
        if (!m_pendingSlotDigits.isEmpty()) {
            focusAppAtSlot(m_pendingSlotDigits.toInt() - 1);
            m_pendingSlotDigits.clear();
        }
    });

    connect(qApp, &QGuiApplication::applicationStateChanged, this,
            [this](Qt::ApplicationState state) {
                if (state == Qt::ApplicationInactive && isVisible()) {
                    hide();
                }
            });

    connect(
        m_ui->LV_openApps->selectionModel(),
        &QItemSelectionModel::currentChanged, this, [this]() {
            m_hasNavigated = true;
            if (m_timerShouldStart) {
                m_selectionCommitTimer->start(
                    Settings::getInstance().selectionCommitTimeoutMs());
            }
        });

    connect(m_ui->LV_openApps, &QListView::clicked, &Util::focusWindowAtIndex);

    connect(m_ui->LV_openApps, &QListView::customContextMenuRequested, this,
            &AppSwitcher::customContextMenuRequested);

    connect(m_ui->PTE_appSearch, &QPlainTextEdit::textChanged, this,
            &AppSwitcher::onTextChanged);

    connect(&TrackedWindows::getInstance(),
            &TrackedWindows::queuedAppActuallyClosed, this,
            &AppSwitcher::removeQueuedAppToClose);
}

AppSwitcher::~AppSwitcher() {
    delete m_ui;
}

void AppSwitcher::onTextChanged() {
    const auto searchQuery = m_ui->PTE_appSearch->toPlainText().trimmed();

    for (int row = 0; row < m_listModel->rowCount(); ++row) {
        const QStandardItem* item = m_listModel->item(row);
        if (!item) {
            continue;
        }

        const bool matches =
            searchQuery.isEmpty() ||
            item->text().contains(searchQuery, Qt::CaseInsensitive);

        m_ui->LV_openApps->setRowHidden(row, !matches);
    }

    int matchCount{0};
    int lastMatchingRow{-1};
    for (int row = 0; row < m_listModel->rowCount(); ++row) {
        if (!m_ui->LV_openApps->isRowHidden(row)) {
            ++matchCount;
            lastMatchingRow = row;
        }
    }

    if (matchCount == 1) {
        const QModelIndex onlyMatch = m_listModel->index(lastMatchingRow, 0);
        m_ui->LV_openApps->setCurrentIndex(onlyMatch);
        m_hasNavigated = true;
    }
}

void AppSwitcher::customContextMenuRequested(const QPoint& pos) {
    const auto index = m_ui->LV_openApps->indexAt(pos);

    if (!index.isValid()) {
        return;
    }

    const auto data = index.data(Constants::ROLE_INTERNAL_LIST_DATA);

    if (data.isValid() && data.canConvert<WindowDetailsInternal>()) {
        const auto idata = data.value<WindowDetailsInternal>();

        const auto appExePath = Util::getFullProcessPath(idata.processId);
        const auto appName = Util::getAppNameFromTitle(idata.title);

        const QFontMetrics metrics(this->font());
        const QString elidedText =
            metrics.elidedText(idata.title, Qt::ElideRight, 128);

        m_actionAddAppRuleByName->setText(
            tr("Add name rule for \"%1\"").arg(appName));
        m_actionAddAppRuleByPath->setText(
            tr("Add path rule for %1").arg(appExePath));
        m_actionTerminateApp->setText(tr("Terminate %1").arg(elidedText));
        m_actionCloseApp->setText(tr("Close %1").arg(elidedText));

        const auto* selected = m_customItemContextMenu->exec(mapToGlobal(pos));

        if (selected == m_actionAddAppRuleByName) {
            qDebug() << "add rule by name selected: ";
        }
        else if (selected == m_actionAddAppRuleByPath) {
            qDebug() << "add rule by path selected";
        }
        else if (selected == m_actionTerminateApp) {
            qDebug() << "Terminating hWnd: " << idata.hWnd;
            Util::terminateAppWithHWND(idata.hWnd);
        }
        else if (selected == m_actionCloseApp) {
            qDebug() << "Closing hWnd: " << idata.hWnd;
            Util::closeAppWithHWND(idata.hWnd);
        }
    }
}

void AppSwitcher::showEvent(QShowEvent* event) {
    m_listModel->clear();

    const auto ordered = TrackedWindows::getInstance().getOrderedWindows();
    const auto windows = TrackedWindows::getInstance().getWindows();

    m_listModel->setRowCount(static_cast<int>(ordered.size()));

    int row{};
    for (HWND hWnd : ordered) {
        if (!hWnd) {
            continue;
        }

        const auto it = windows.constFind(hWnd);
        if (it == windows.constEnd()) {
            continue;
        }

        const WindowDetails& wDetails = it.value();

        auto* item = new QStandardItem();

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
                      Constants::ROLE_INTERNAL_LIST_DATA);
        item->setData(TrackedWindows::getInstance().getWindowOrder(hWnd),
                      Constants::ROLE_SLOT_INDEX);

        m_listModel->setItem(row, item);
        ++row;
    }
    m_listModel->setRowCount(row);

    const auto mru = TrackedWindows::getInstance().getMRUOrder();
    if (mru.size() >= 2) {
        HWND target = mru.at(1);
        for (int row = 0; row < m_listModel->rowCount(); ++row) {
            const auto* item = m_listModel->item(row);
            if (!item) {
                continue;
            }

            const auto data = item->data(Constants::ROLE_INTERNAL_LIST_DATA);
            if (data.canConvert<WindowDetailsInternal>() &&
                data.value<WindowDetailsInternal>().hWnd == target)
            {
                m_ui->LV_openApps->setCurrentIndex(m_listModel->index(row, 0));
                break;
            }
        }
    }

    m_ui->LV_openApps->setFocus();
    m_hasNavigated = false;
    m_timerShouldStart = false;
    if (m_selectionCommitTimer->isActive()) {
        m_selectionCommitTimer->stop();
    }

    qApp->installEventFilter(this);

    QWidget::showEvent(event);
}

void AppSwitcher::hideEvent(QHideEvent* event) {
    m_selectionCommitTimer->stop();
    qApp->removeEventFilter(this);
    WinProcs::setSwitcherOpen(false);

    if (!m_ui->PTE_appSearch->toPlainText().isEmpty()) {
        m_ui->PTE_appSearch->clear();
    }

    QWidget::hideEvent(event);
}

bool AppSwitcher::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_ui->LV_openApps->viewport() &&
        event->type() == QEvent::Drop)
    {
        handleAppReorder(dynamic_cast<QDropEvent*>(event));
        return true;
    }

    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        const int key = keyEvent->key();
        const int normalizedKey = (key == Qt::Key_Backtab) ? Qt::Key_Tab : key;
        const QKeySequence pressed(keyEvent->modifiers() | normalizedKey);

        if (Settings::getInstance().cycleForwardShortcuts().contains(pressed)) {
            m_timerShouldStart = true;
            cycleSelection();
            return true;
        }

        if (Settings::getInstance().cycleBackwardShortcuts().contains(pressed))
        {
            m_timerShouldStart = true;
            cycleSelectionBackward();
            return true;
        }

        if (Settings::getInstance()
                .activateSelectionAndHideShortcuts()
                .contains(pressed))
        {
            activateSelectionAndHide();
            return true;
        }

        if (Settings::getInstance().hideAppSwitcherShortcuts().contains(
                pressed))
        {
            if (m_ui->PTE_appSearch->hasFocus()) {
                m_ui->PTE_appSearch->clear();
                m_ui->PTE_appSearch->clearFocus();
                return true;
            }

            hide();
            return true;
        }

        if (!m_ui->PTE_appSearch->hasFocus()) {
            if (key == Qt::Key_0) {
                focusAppAtSlot(9);
                m_pendingSlotDigits.clear();
                m_slotInputTimer->stop();
                return true;
            }

            if (key >= Qt::Key_1 && key <= Qt::Key_9) {
                m_pendingSlotDigits += QChar('1' + (key - Qt::Key_1));

                const int rowCount = m_listModel->rowCount();
                const int typedValue = m_pendingSlotDigits.toInt();

                if (m_pendingSlotDigits.length() >= 2 ||
                    typedValue * 10 > rowCount)
                {
                    focusAppAtSlot(typedValue - 1);
                    m_pendingSlotDigits.clear();
                    m_slotInputTimer->stop();
                }
                else {
                    m_slotInputTimer->start(500);
                }

                return true;
            }

            if (key >= Qt::Key_A && key <= Qt::Key_Z) {
                m_ui->PTE_appSearch->setFocus();
                QTextCursor cursor = m_ui->PTE_appSearch->textCursor();
                cursor.movePosition(QTextCursor::End);
                cursor.insertText(keyEvent->text());
                m_ui->PTE_appSearch->setTextCursor(cursor);
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void AppSwitcher::removeQueuedAppToClose(int slot) {
    for (int row = 0; row < m_listModel->rowCount(); ++row) {
        auto* item = m_listModel->item(row);
        if (item && item->data(Constants::ROLE_SLOT_INDEX).toInt() == slot) {
            m_listModel->removeRow(row);
            break;
        }
    }
}

void AppSwitcher::altReleased(bool wasQuickTap) {
    if (wasQuickTap || m_hasNavigated) {
        activateSelectionAndHide();
    }
}

void AppSwitcher::handleAppReorder(QDropEvent* event) {
    event->setDropAction(Qt::CopyAction);
    const QModelIndex fromIndex = m_ui->LV_openApps->currentIndex();
    if (!fromIndex.isValid()) {
        return;
    }

    const QModelIndex targetIndex =
        m_ui->LV_openApps->indexAt(event->position().toPoint());
    int toRow =
        targetIndex.isValid() ? targetIndex.row() : m_listModel->rowCount() - 1;
    const int fromRow = fromIndex.row();

    if (targetIndex.isValid()) {
        const QRect targetRect = m_ui->LV_openApps->visualRect(targetIndex);
        const bool droppedInLowerHalf =
            event->position().y() > targetRect.center().y();
        if (droppedInLowerHalf) {
            ++toRow;
        }
    }

    if (fromRow == toRow) {
        event->ignore();
        return;
    }

    const QList<QStandardItem*> movedRow = m_listModel->takeRow(fromRow);
    if (toRow > fromRow) {
        --toRow;
    }
    m_listModel->insertRow(toRow, movedRow);

    QVector<HWND> newOrder;
    newOrder.reserve(m_listModel->rowCount());
    for (int row = 0; row < m_listModel->rowCount(); ++row) {
        const auto data =
            m_listModel->item(row)->data(Constants::ROLE_INTERNAL_LIST_DATA);
        newOrder.push_back(data.value<WindowDetailsInternal>().hWnd);
    }
    TrackedWindows::getInstance().reorderSlots(newOrder);

    for (int row = 0; row < m_listModel->rowCount(); ++row) {
        m_listModel->item(row)->setData(row, Constants::ROLE_SLOT_INDEX);
    }

    m_ui->LV_openApps->setCurrentIndex(m_listModel->index(toRow, 0));
    m_timerShouldStart = false;

    event->accept();
}

void AppSwitcher::focusAppAtSlot(int slot) {
    const auto* model = m_ui->LV_openApps->model();

    for (int row = 0; row < model->rowCount(); ++row) {
        const QModelIndex index = model->index(row, 0);
        if (index.data(Constants::ROLE_SLOT_INDEX).toInt() != slot) {
            continue;
        }

        const QVariant data = index.data(Constants::ROLE_INTERNAL_LIST_DATA);
        if (data.isValid() && data.canConvert<WindowDetailsInternal>()) {
            const auto idata = data.value<WindowDetailsInternal>();

            Util::focusWindowWithHWND(idata.hWnd);
        }
        return;
    }
}

void AppSwitcher::cycleSelection() {
    auto* model = m_ui->LV_openApps->model();
    const int rowCount = model->rowCount();
    if (rowCount == 0) {
        return;
    }

    int currentRow = m_ui->LV_openApps->currentIndex().row();
    if (currentRow < 0) {
        currentRow = -1;
    }

    int nextRow = currentRow;
    bool found = false;
    for (int i = 0; i < rowCount; ++i) {
        nextRow = (nextRow + 1) % rowCount;
        if (!m_ui->LV_openApps->isRowHidden(nextRow)) {
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }

    const QModelIndex nextIndex = model->index(nextRow, 0);
    m_ui->LV_openApps->setFocus();
    m_ui->LV_openApps->setCurrentIndex(nextIndex);
    m_ui->LV_openApps->selectionModel()->select(
        nextIndex, QItemSelectionModel::ClearAndSelect);
    m_ui->LV_openApps->viewport()->update();
    m_hasNavigated = true;
}

void AppSwitcher::cycleSelectionBackward() {
    auto* model = m_ui->LV_openApps->model();
    const int rowCount = model->rowCount();
    if (rowCount == 0) {
        return;
    }

    int prevRow{};
    bool needsAdvance{true};

    if (!m_hasNavigated) {
        const int currentRow = m_ui->LV_openApps->currentIndex().row();
        if (currentRow >= 0 && !m_ui->LV_openApps->isRowHidden(currentRow)) {
            prevRow = currentRow;
            needsAdvance = false;
        }
        else {
            prevRow = 0;
        }
    }
    else {
        prevRow = m_ui->LV_openApps->currentIndex().row();
    }

    if (needsAdvance) {
        bool found{};
        for (int i = 0; i < rowCount; ++i) {
            prevRow = (prevRow - 1 + rowCount) % rowCount;
            if (!m_ui->LV_openApps->isRowHidden(prevRow)) {
                found = true;
                break;
            }
        }
        if (!found) {
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

void AppSwitcher::activateSelectionAndHide() {
    m_selectionCommitTimer->stop();

    const QModelIndex idx = m_ui->LV_openApps->currentIndex();
    if (idx.isValid()) {
        const QVariant data = idx.data(Constants::ROLE_INTERNAL_LIST_DATA);
        if (data.isValid() && data.canConvert<WindowDetailsInternal>()) {
            Util::focusWindowWithHWND(data.value<WindowDetailsInternal>().hWnd);
        }
    }

    hide();
}
