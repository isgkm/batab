#include "keyboardsettingswidget.h"

#include "constants.h"
#include "settings.h"
#include "ui_keyboardsettingswidget.h"

#include <QAction>
#include <QKeySequenceEdit>
#include <QMenu>
#include <QPushButton>
#include <QVector>

void KeyboardSettingsWidget::updateTableData() {
    const auto allShortcuts = Settings::getInstance().collectAllShortcuts();

    m_ui->TW_shortcuts->setRowCount(static_cast<int>(allShortcuts.size()));

    int it = 0;
    for (const auto& [key, val] : allShortcuts.asKeyValueRange()) {
        auto* actionName = new QTableWidgetItem(key.first);
        actionName->setData(Constants::ROLE_SHORTCUT_FOR_ACTION,
                            QVariant::fromValue(key.second));

        auto* actionShortcuts = new QTableWidgetItem(val.join(", "));

        m_ui->TW_shortcuts->setItem(it, 0, actionName);
        m_ui->TW_shortcuts->setItem(it, 1, actionShortcuts);
        ++it;
    }
}

KeyboardSettingsWidget::KeyboardSettingsWidget(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::KeyboardSettingsWidget) {
    m_ui->setupUi(this);

    m_ui->TW_shortcuts->setStyleSheet(
        "QTableWidget::item:selected { background-color: #3d5afe; color: "
        "white; border: none; outline: none; }");
    m_ui->TW_shortcuts->setColumnCount(2);
    m_ui->TW_shortcuts->setHorizontalHeaderLabels(
        {tr("Action"), tr("Shortcut")});
    m_ui->TW_shortcuts->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    m_ui->TW_shortcuts->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);
    m_ui->TW_shortcuts->verticalHeader()->setVisible(false);
    m_ui->TW_shortcuts->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->TW_shortcuts->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->TW_shortcuts->setSelectionMode(QAbstractItemView::SingleSelection);

    m_ui->GB_shortcutManager->hide();

    updateTableData();

    connect(
        m_ui->TW_shortcuts, &QTableWidget::itemSelectionChanged, this,
        [this]() {
            const auto currentRow = m_ui->TW_shortcuts->currentRow();
            if (currentRow < 0) {
                return;
            }

            const auto* shortcuts = m_ui->TW_shortcuts->item(currentRow, 1);
            const auto* action = m_ui->TW_shortcuts->item(currentRow, 0);
            auto shortcutsFor =
                action->data(Constants::ROLE_SHORTCUT_FOR_ACTION)
                    .value<ShortcutsFor>();

            auto* groupBoxLayout =
                qobject_cast<QGridLayout*>(m_ui->GB_shortcutManager->layout());

            m_ui->GB_shortcutManager->setTitle(action->text());
            m_ui->GB_shortcutManager->show();

            for (auto* widget : std::as_const(m_queuedToDelete)) {
                groupBoxLayout->removeWidget(widget);
                widget->deleteLater();
            }
            m_queuedToDelete.clear();

            int row{0};
            for (const auto& shortcut :
                 shortcuts->text().split(", ", Qt::SkipEmptyParts))
            {
                auto* ksq = new QKeySequenceEdit(shortcut);
                ksq->setClearButtonEnabled(true);

                auto* removeButton = new QPushButton("Remove");

                groupBoxLayout->addWidget(ksq, row, 0);
                groupBoxLayout->addWidget(removeButton, row, 1);

                connect(
                    removeButton, &QPushButton::clicked, this,
                    [this, groupBoxLayout, ksq, removeButton, shortcutsFor] {
                        groupBoxLayout->removeWidget(ksq);
                        groupBoxLayout->removeWidget(removeButton);

                        m_queuedToDelete.removeOne(ksq);
                        m_queuedToDelete.removeOne(removeButton);

                        ksq->deleteLater();
                        removeButton->deleteLater();

                        Settings::getInstance().manageShortcutsFor(
                            shortcutsFor, ShortcutAction::REMOVE,
                            ksq->keySequence());

                        updateTableData();
                    });

                m_queuedToDelete.append(ksq);
                m_queuedToDelete.append(removeButton);
                ++row;
            }

            auto* addButton = new QPushButton("Add");
            groupBoxLayout->addWidget(addButton, row, 1);
            m_queuedToDelete.append(addButton);

            connect(addButton, &QPushButton::clicked, this,
                    [this, groupBoxLayout, addButton, shortcutsFor] {
                        auto* keySequence = new QKeySequenceEdit();
                        keySequence->setClearButtonEnabled(true);

                        auto* removeButton = new QPushButton("Remove");

                        const int index = groupBoxLayout->indexOf(addButton);
                        int row{};
                        int col{};
                        int rowSpan{};
                        int colSpan{};
                        groupBoxLayout->getItemPosition(index, &row, &col,
                                                        &rowSpan, &colSpan);

                        groupBoxLayout->removeWidget(addButton);

                        groupBoxLayout->addWidget(keySequence, row, 0);
                        groupBoxLayout->addWidget(removeButton, row, 1);
                        groupBoxLayout->addWidget(addButton, row + 1, 1);

                        connect(keySequence, &QKeySequenceEdit::editingFinished,
                                this, [this, keySequence, shortcutsFor]() {
                                    Settings::getInstance().manageShortcutsFor(
                                        shortcutsFor, ShortcutAction::ADD,
                                        keySequence->keySequence());
                                    updateTableData();
                                });

                        connect(removeButton, &QPushButton::clicked, this,
                                [this, groupBoxLayout, keySequence,
                                 removeButton, shortcutsFor] {
                                    groupBoxLayout->removeWidget(keySequence);
                                    groupBoxLayout->removeWidget(removeButton);

                                    m_queuedToDelete.removeOne(keySequence);
                                    m_queuedToDelete.removeOne(removeButton);

                                    keySequence->deleteLater();
                                    removeButton->deleteLater();

                                    Settings::getInstance().manageShortcutsFor(
                                        shortcutsFor, ShortcutAction::REMOVE,
                                        keySequence->keySequence());
                                    updateTableData();
                                });

                        m_queuedToDelete.append(keySequence);
                        m_queuedToDelete.append(removeButton);
                    });
        });
}

KeyboardSettingsWidget::~KeyboardSettingsWidget() {
    delete m_ui;
}