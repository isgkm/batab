#include "appswitcher.h"
#include "trackedwindows.h"
#include "ui_appswitcher.h"
#include "util.h"

AppSwitcher::AppSwitcher(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AppSwitcher)
    , listModel(new QStandardItemModel())
    // , shownApps(new QList<HWND>())
{
    ui->setupUi(this);

    this->ui->LV_openApps->setModel(this->listModel);

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

    for (const auto &[key, wDetails] :
         TrackedWindows::getInstance()->getWindows().asKeyValueRange()) {
        // if (shownApps->contains(key))
        //     continue;

        QStandardItem *item = new QStandardItem();

        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
        item->setEditable(false);
        item->setToolTip(wDetails.title);

        auto len = wDetails.title.length();
        item->setText(len > 50 ? wDetails.title.mid(0, 50) + "..." : wDetails.title);

        item->setIcon(wDetails.icon);

        WindowDetailsInternal wdi{.hWnd = key, .title = wDetails.title, .PID = wDetails.PID};

        item->setData(QVariant::fromValue(wdi), InternalListDataRole);

        this->listModel->appendRow(item);
        // this->shownApps->append(key);
        // this->listModel->insertRow(wDetails.listOrder - 1, item);
    }

    QWidget::showEvent(event);
}

// void AppSwitcher::hideEvent(QHideEvent *event)
// {

//     QWidget::hideEvent(event);
// }

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
        key -= Qt::Key_0;
        auto data = this->ui->LV_openApps->model()
                        ->index(key == Qt::Key_0 ? 10 : key - 1, 0)
                        .data(InternalListDataRole);

        if (data.isValid() && data.canConvert<WindowDetailsInternal>()) {
            auto idata = data.value<WindowDetailsInternal>();

            qDebug() << "Clicked pid: " << idata.PID << " hwnd: " << idata.hWnd
            << " title: " << idata.title;

            Util::focusWindowWithHWND(idata.hWnd);
        }
    }
}