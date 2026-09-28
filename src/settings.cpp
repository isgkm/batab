#include "settings.h"

#include "constants.h"

#include <QCoreApplication>
#include <QDir>
#include <QKeySequence>

#include <windows.h>

Settings::Settings()
    : m_settings(QSettings::IniFormat, QSettings::UserScope, "Batab",
                 "settings") {
}

Settings& Settings::getInstance() {
    static Settings s_instance;

    return s_instance;
}

bool Settings::launchOnSystemStartup() const {
    return m_settings
        .value(Constants::SETTINGS_KEY_LAUNCH_ON_SYSTEM_STARTUP, false)
        .toBool();
}

void Settings::setLaunchOnSystemStartup(bool enabled) {
    QSettings registrySettings(Constants::REGKEY_WINDOWS_AUTOSTART,
                               QSettings::NativeFormat);
    const auto appName = QCoreApplication::applicationName();

    if (enabled) {
        const auto exePath =
            QDir::toNativeSeparators(QCoreApplication::applicationFilePath());

        registrySettings.setValue(appName, QString("\"%1\"").arg(exePath));
    }
    else {
        registrySettings.remove(appName);
    }

    m_settings.setValue(Constants::SETTINGS_KEY_LAUNCH_ON_SYSTEM_STARTUP,
                        enabled);
}

int Settings::selectionCommitTimeoutMs() const {
    return m_settings
        .value(Constants::SETTINGS_KEY_SELECTION_COMMIT_TIMEOUT_MS, 1000)
        .toInt();
}

void Settings::setSelectionCommitTimeoutMs(int timeMs) {
    m_settings.setValue(Constants::SETTINGS_KEY_SELECTION_COMMIT_TIMEOUT_MS,
                        timeMs);
}

bool Settings::hotCornerEnabled() const {
    return m_settings.value(Constants::SETTINGS_KEY_HOT_CORNER_ENABLED, false)
        .toBool();
}

void Settings::setHotCornerEnabled(bool option) {
    m_settings.setValue(Constants::SETTINGS_KEY_HOT_CORNER_ENABLED, option);
}

HotCornerTriggerArea Settings::hotCornerTriggerArea() const {
    return static_cast<HotCornerTriggerArea>(
        m_settings
            .value(Constants::SETTINGS_KEY_HOT_CORNER_TRIGGER_AREA,
                   static_cast<int>(HotCornerTriggerArea::TopLeft))
            .toInt());
}

void Settings::setHotCornerTriggerArea(HotCornerTriggerArea option) {
    m_settings.setValue(Constants::SETTINGS_KEY_HOT_CORNER_TRIGGER_AREA,
                        static_cast<int>(option));
}

int Settings::hotCornerSize() const {
    return m_settings.value(Constants::SETTINGS_KEY_HOT_CORNER_SIZE, 32)
        .toInt();
};

void Settings::setHotCornerSize(int sizePx) {
    m_settings.setValue(Constants::SETTINGS_KEY_HOT_CORNER_SIZE, sizePx);
}

int Settings::hotCornerRequiredTimeMs() const {
    return m_settings
        .value(Constants::SETTINGS_KEY_HOT_CORNER_REQUIRED_TIME_MS, 1000)
        .toInt();
}

void Settings::setHotCornerRequiredTimeMs(int timeMs) {
    m_settings.setValue(Constants::SETTINGS_KEY_HOT_CORNER_REQUIRED_TIME_MS,
                        timeMs);
}

int Settings::quickSwitchHoldThresholdMs() const {
    return m_settings
        .value(Constants::SETTINGS_KEY_QUICK_SWITCH_HOLD_THRESHOLD_MS, 100)
        .toInt();
}

void Settings::setQuickSwitchHoldThresholdMs(int timeMs) {
    m_settings.setValue(Constants::SETTINGS_KEY_QUICK_SWITCH_HOLD_THRESHOLD_MS,
                        timeMs);
}

bool Settings::isMouseInHotCornerArea(const POINT& cursorPos,
                                      HotCornerTriggerArea triggerArea) {
    HMONITOR monitor = MonitorFromPoint(cursorPos, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo{};
    monitorInfo.cbSize = sizeof(MONITORINFO);

    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        return false;
    }

    const RECT& bounds = monitorInfo.rcMonitor;
    const int hotCornerSize = Settings::getInstance().hotCornerSize();

    const int relX = cursorPos.x - bounds.left;
    const int relY = cursorPos.y - bounds.top;
    const int monitorWidth = bounds.right - bounds.left;
    const int monitorHeight = bounds.bottom - bounds.top;

    switch (triggerArea) {
        case HotCornerTriggerArea::TopLeft:
            return relX < hotCornerSize && relY < hotCornerSize;
        case HotCornerTriggerArea::TopRight:
            return relX >= monitorWidth - hotCornerSize && relY < hotCornerSize;
        case HotCornerTriggerArea::BottomLeft:
            return relX < hotCornerSize &&
                   relY >= monitorHeight - hotCornerSize;
        case HotCornerTriggerArea::BottomRight:
            return relX >= monitorWidth - hotCornerSize &&
                   relY >= monitorHeight - hotCornerSize;
        case HotCornerTriggerArea::Top:
            return relY < hotCornerSize;
        case HotCornerTriggerArea::Bottom:
            return relY >= monitorHeight - hotCornerSize;
        case HotCornerTriggerArea::Left:
            return relX < hotCornerSize;
        case HotCornerTriggerArea::Right:
            return relX >= monitorWidth - hotCornerSize;
    }

    return false;
}

QStringList Settings::qKeySequenceListToQStringList(
    const QList<QKeySequence>& qKeySequenceList) {
    QStringList raw;

    for (const auto& seq : qKeySequenceList) {
        raw.append(seq.toString());
    }

    return raw;
}

QList<QKeySequence> Settings::cycleForwardShortcuts() const {
    const QStringList raw =
        m_settings
            .value(Constants::SETTINGS_KEY_CYCLE_FORWARD_SHORTCUTS,
                   Constants::DEFAULT_SHORTCUTS_CYCLE_FORWARD)
            .toStringList();

    QList<QKeySequence> result;
    for (const auto& shortcut : raw) {
        result.append(QKeySequence(shortcut));
    }

    return result;
}

QList<QKeySequence> Settings::cycleBackwardShortcuts() const {
    const QStringList raw =
        m_settings
            .value(Constants::SETTINGS_KEY_CYCLE_BACKWARD_SHORTCUTS,
                   Constants::DEFAULT_SHORTCUTS_CYCLE_BACKWARD)
            .toStringList();

    QList<QKeySequence> result;
    for (const auto& shortcut : raw) {
        result.append(QKeySequence(shortcut));
    }

    return result;
}

QList<QKeySequence> Settings::activateSelectionAndHideShortcuts() const {
    const QStringList raw =
        m_settings
            .value(Constants::SETTINGS_KEY_ACTIVATE_SELECTION_AND_HIDE,
                   Constants::DEFAULT_SHORTCUTS_ACTIVATE_SELECTION_AND_HIDE)
            .toStringList();

    QList<QKeySequence> result;
    for (const auto& shortcut : raw) {
        result.append(QKeySequence(shortcut));
    }

    return result;
}

QList<QKeySequence> Settings::hideAppSwitcherShortcuts() const {
    const QStringList raw =
        m_settings
            .value(Constants::SETTINGS_KEY_HIDE_APP_SWITCHER,
                   Constants::DEFAULT_SHORTCUTS_HIDE_APP_SWITCHER)
            .toStringList();

    QList<QKeySequence> result;
    for (const auto& shortcut : raw) {
        result.append(QKeySequence(shortcut));
    }

    return result;
}

QMap<QPair<QString, ShortcutsFor>, QStringList> Settings::collectAllShortcuts()
    const {
    QMap<QPair<QString, ShortcutsFor>, QStringList> ret;

    auto toQStringList = [](const QList<QKeySequence>& list) {
        QStringList res;
        for (const auto& el : list) {
            res.append(el.toString());
        }

        return res;
    };

    ret.insert({QStringLiteral("Cycle Forward"), ShortcutsFor::CYCLE_FORWARD},
               toQStringList(cycleForwardShortcuts()));
    ret.insert({QStringLiteral("Cycle Backward"), ShortcutsFor::CYCLE_BACKWARD},
               toQStringList(cycleBackwardShortcuts()));
    ret.insert({QStringLiteral("Activate Selection and Hide"),
                ShortcutsFor::ACTIVATE_SELECTION_AND_HIDE},
               toQStringList(activateSelectionAndHideShortcuts()));
    ret.insert(
        {QStringLiteral("Hide App Switcher"), ShortcutsFor::HIDE_APP_SWITCHER},
        toQStringList(hideAppSwitcherShortcuts()));

    return ret;
}

void Settings::manageShortcutsFor(ShortcutsFor whichShortcut,
                                  ShortcutAction whichAction,
                                  const QKeySequence& sequence) {
    QList<QKeySequence> currentShortcuts;
    QLatin1StringView settingsKey;

    switch (whichShortcut) {
        case ShortcutsFor::CYCLE_FORWARD:
            currentShortcuts = cycleForwardShortcuts();
            settingsKey = Constants::SETTINGS_KEY_CYCLE_FORWARD_SHORTCUTS;
            break;
        case ShortcutsFor::CYCLE_BACKWARD:
            currentShortcuts = cycleBackwardShortcuts();
            settingsKey = Constants::SETTINGS_KEY_CYCLE_BACKWARD_SHORTCUTS;
            break;
        case ShortcutsFor::ACTIVATE_SELECTION_AND_HIDE:
            currentShortcuts = activateSelectionAndHideShortcuts();
            settingsKey = Constants::SETTINGS_KEY_ACTIVATE_SELECTION_AND_HIDE;
            break;
        case ShortcutsFor::HIDE_APP_SWITCHER:
            currentShortcuts = hideAppSwitcherShortcuts();
            settingsKey = Constants::SETTINGS_KEY_HIDE_APP_SWITCHER;
            break;
    }

    if (whichAction == ShortcutAction::ADD) {
        if (!currentShortcuts.contains(sequence)) {
            currentShortcuts.append(sequence);
        }
    }
    else {
        currentShortcuts.removeOne(sequence);
    }

    m_settings.setValue(settingsKey,
                        qKeySequenceListToQStringList(currentShortcuts));
}
