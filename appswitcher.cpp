#include "appswitcher.h"

#include "indexedicondelegate.h"
#include "trackedwindows.h"
#include "ui_appswitcher.h"
#include "util.h"

AppSwitcher::AppSwitcher(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AppSwitcher)
    , listModel(new QStandardItemModel())
{
    ui->setupUi(this);

    this->ui->LV_openApps->setModel(this->listModel);
    this->ui->LV_openApps->setItemDelegate(new IndexedIconDelegate(this));

    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setFixedSize(400, 500);

    qDebug() << "appswitcher ct";

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
        item->setText(len > 50 ? wDetails.title.left(50) + "..."
                               : wDetails.title);

        item->setIcon(wDetails.icon);

        WindowDetailsInternal wdi{
            .hWnd = hWnd, .title = wDetails.title, .PID = wDetails.PID};

        item->setData(QVariant::fromValue(wdi), InternalListDataRole);
        item->setData(trackedWindows->getWindowOrder(hWnd), SlotIndexRole);

        this->listModel->setItem(row, item);
        ++row;
    }

    QWidget::showEvent(event);
}

void AppSwitcher::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "press: " << event->key() << " with modifiers: " << event->modifiers();
}

void AppSwitcher::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "release: " << event->key() << " with modifiers: " << event->modifiers();

    auto key = event->key();
    if (key == Qt::Key_Tab) {
        qDebug() << "tab pressed ";

        auto model = this->ui->LV_openApps->model();
        int rowCount = model->rowCount();

        if (rowCount > 0) {
            int currentRow = this->ui->LV_openApps->currentIndex().row();

            int nextRow = (currentRow + 1) % rowCount;

            auto nextIdx = model->index(nextRow, 0);

            this->ui->LV_openApps->setFocus();
            this->ui->LV_openApps->setCurrentIndex(nextIdx);
            this->ui->LV_openApps->selectionModel()->select(nextIdx,
                                                            QItemSelectionModel::ClearAndSelect);

             // Util::focusWindowAtIndex(nextIdx);
        }
    } else if (key >= Qt::Key_1 && key <= Qt::Key_9) {
        const int typedSlot = key - Qt::Key_1;

        auto *model = ui->LV_openApps->model();

        for (int row = 0; row < model->rowCount(); ++row)
        {
            const QModelIndex index = model->index(row, 0);
            if (index.data(SlotIndexRole).toInt() != typedSlot)
                continue;

            const QVariant data = index.data(InternalListDataRole);
            if (data.isValid() && data.canConvert<WindowDetailsInternal>())
            {
                const auto idata = data.value<WindowDetailsInternal>();

                qDebug() << "Clicked pid: " << idata.PID
                         << " hwnd: " << idata.hWnd
                         << " title: " << idata.title;

                Util::focusWindowWithHWND(idata.hWnd);
            }
        }
    }
}