#include "generalsettingswidget.h"

#include "constants.h"
#include "settings.h"
#include "ui_generalsettingswidget.h"

#include <QPushButton>

GeneralSettingsWidget::GeneralSettingsWidget(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::GeneralSettingsWidget) {
    m_ui->setupUi(this);
    populateHotCornerComboboxOptions();

    m_ui->CB_launchOnSystemStartup->setChecked(
        Settings::getInstance().launchOnSystemStartup());

    connect(m_ui->CB_launchOnSystemStartup, &QCheckBox::checkStateChanged, this,
            [](Qt::CheckState state) {
                qDebug() << "[ DEBUG ] launchOnSystemStartup set to: " << state;
                Settings::getInstance().setLaunchOnSystemStartup(
                    state == Qt::CheckState::Checked ? true : false);
            });

    m_ui->SB_commitTimeMs->setValue(
        Settings::getInstance().selectionCommitTimeoutMs());

    connect(m_ui->SB_commitTimeMs, &QSpinBox::valueChanged, this,
            [](int newVal) {
                qDebug() << "[ DEBUG ] commitTimeMs set to: " << newVal;
                Settings::getInstance().setSelectionCommitTimeoutMs(newVal);
            });

    m_ui->SB_quickSwitchTimeMs->setValue(
        Settings::getInstance().quickSwitchHoldThresholdMs());

    connect(m_ui->SB_quickSwitchTimeMs, &QSpinBox::valueChanged, this,
            [](int newVal) {
                qDebug() << "[ DEBUG ] quickSwitchTimeMs set to: " << newVal;
                Settings::getInstance().setQuickSwitchHoldThresholdMs(newVal);
            });

    const auto hotCornerEnabled = Settings::getInstance().hotCornerEnabled();
    m_ui->CB_hotCornerEnabled->setCheckState(
        hotCornerEnabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    m_ui->W_hotCornerSettings->setEnabled(hotCornerEnabled);
    connect(m_ui->CB_hotCornerEnabled, &QCheckBox::checkStateChanged, this,
            [this](Qt::CheckState state) {
                qDebug() << "[ DEBUG ] hotCornerEnabled set to: " << state;
                if (state == Qt::Unchecked) {
                    m_ui->W_hotCornerSettings->setEnabled(false);
                    Settings::getInstance().setHotCornerEnabled(false);
                }
                else {
                    m_ui->W_hotCornerSettings->setEnabled(true);
                    Settings::getInstance().setHotCornerEnabled(true);
                }
            });

    const auto hotCornerTriggerArea =
        static_cast<int>(Settings::getInstance().hotCornerTriggerArea());
    const auto idx =
        m_ui->CMB_hotCornerTriggerArea->findData(hotCornerTriggerArea);
    if (idx >= 0) {
        m_ui->CMB_hotCornerTriggerArea->setCurrentIndex(idx);
    }

    connect(m_ui->CMB_hotCornerTriggerArea, &QComboBox::currentIndexChanged,
            this, [this](int newIdx) {
                qDebug() << "[ DEBUG ] hotCornerTriggerArea set to: " << newIdx;
                const auto value =
                    m_ui->CMB_hotCornerTriggerArea->itemData(newIdx).toInt();
                Settings::getInstance().setHotCornerTriggerArea(
                    static_cast<HotCornerTriggerArea>(value));
            });

    m_ui->SB_hotCornerSize->setValue(Settings::getInstance().hotCornerSize());

    connect(m_ui->SB_hotCornerSize, &QSpinBox::valueChanged, this,
            [](int newValue) {
                qDebug() << "[ DEBUG ] hotCornerSize set to: " << newValue;
                Settings::getInstance().setHotCornerSize(newValue);
            });

    m_ui->SB_hotCornerRequiredTimeMs->setValue(
        Settings::getInstance().hotCornerRequiredTimeMs());
    connect(m_ui->SB_hotCornerRequiredTimeMs, &QSpinBox::valueChanged, this,
            [](int newValue) {
                qDebug() << "[ DEBUG ] hotCornerRequiredTimeMs set to: "
                         << newValue;
                Settings::getInstance().setHotCornerRequiredTimeMs(newValue);
            });

    connect(m_ui->BTN_reset, &QPushButton::clicked, this, [this] {
        m_ui->CB_launchOnSystemStartup->setChecked(
            Constants::DEFAULT_VALUE_LAUNCH_ON_SYSTEM_STARTUP);

        m_ui->SB_commitTimeMs->setValue(
            Constants::DEFAULT_VALUE_SELECTION_COMMIT_TIMEOUT_MS);

        m_ui->SB_quickSwitchTimeMs->setValue(
            Constants::DEFAULT_VALUE_QUICK_SWITCH_HOLD_THRESHOLD_MS);

        m_ui->CB_hotCornerEnabled->setChecked(
            Constants::DEFAULT_VALUE_HOT_CORNER_ENABLED);

        m_ui->CMB_hotCornerTriggerArea->setCurrentIndex(
            static_cast<int>(Constants::DEFAULT_VALUE_HOT_CORNER_TRIGGER_AREA));

        m_ui->SB_hotCornerSize->setValue(
            Constants::DEFAULT_VALUE_HOT_CORNER_SIZE);

        m_ui->SB_hotCornerRequiredTimeMs->setValue(
            Constants::DEFAULT_VALUE_HOT_CORNER_REQUIRED_TIME_MS);
    });
}

GeneralSettingsWidget::~GeneralSettingsWidget() {
    delete m_ui;
}

void GeneralSettingsWidget::populateHotCornerComboboxOptions() {
    m_ui->CMB_hotCornerTriggerArea->addItem(
        tr("Top Left"),
        QVariant::fromValue(static_cast<int>(HotCornerTriggerArea::TopLeft)));
    m_ui->CMB_hotCornerTriggerArea->addItem(
        tr("Top Edge"),
        QVariant::fromValue(static_cast<int>(HotCornerTriggerArea::Top)));
    m_ui->CMB_hotCornerTriggerArea->addItem(
        tr("Top Right"),
        QVariant::fromValue(static_cast<int>(HotCornerTriggerArea::TopRight)));
    m_ui->CMB_hotCornerTriggerArea->addItem(
        tr("Right Edge"),
        QVariant::fromValue(static_cast<int>(HotCornerTriggerArea::Right)));
    m_ui->CMB_hotCornerTriggerArea->addItem(
        tr("Bottom Right"), QVariant::fromValue(static_cast<int>(
                                HotCornerTriggerArea::BottomRight)));
    m_ui->CMB_hotCornerTriggerArea->addItem(
        tr("Bottom Edge"),
        QVariant::fromValue(static_cast<int>(HotCornerTriggerArea::Bottom)));
    m_ui->CMB_hotCornerTriggerArea->addItem(
        tr("Bottom Left"), QVariant::fromValue(static_cast<int>(
                               HotCornerTriggerArea::BottomLeft)));
    m_ui->CMB_hotCornerTriggerArea->addItem(
        tr("Left Edge"),
        QVariant::fromValue(static_cast<int>(HotCornerTriggerArea::Left)));
}
