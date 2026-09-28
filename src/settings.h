#ifndef SETTINGS_H
#define SETTINGS_H

#include "hotcornertriggerarea.h"

#include <QSettings>
#include <QtClassHelperMacros>

#include <windef.h>

enum class ShortcutAction : uint8_t { ADD, REMOVE };

enum class ShortcutsFor : uint8_t {
    CYCLE_FORWARD,
    CYCLE_BACKWARD,
    ACTIVATE_SELECTION_AND_HIDE,
    HIDE_APP_SWITCHER
};
Q_DECLARE_METATYPE(ShortcutsFor)

class Settings final {
  public:
    Q_DISABLE_COPY_MOVE(Settings)

    static Settings& getInstance();

    [[nodiscard]] bool launchOnSystemStartup() const;
    void setLaunchOnSystemStartup(bool enabled);

    [[nodiscard]] int selectionCommitTimeoutMs() const;
    void setSelectionCommitTimeoutMs(int timeMs);

    [[nodiscard]] bool hotCornerEnabled() const;
    void setHotCornerEnabled(bool option);

    [[nodiscard]] HotCornerTriggerArea hotCornerTriggerArea() const;
    void setHotCornerTriggerArea(HotCornerTriggerArea option);

    [[nodiscard]] int hotCornerSize() const;
    void setHotCornerSize(int sizePx);

    [[nodiscard]] int hotCornerRequiredTimeMs() const;
    void setHotCornerRequiredTimeMs(int timeMs);

    [[nodiscard]] int quickSwitchHoldThresholdMs() const;
    void setQuickSwitchHoldThresholdMs(int timeMs);

    [[nodiscard]] static bool isMouseInHotCornerArea(
        const POINT& cursorPos, HotCornerTriggerArea triggerArea);

    [[nodiscard]] QList<QKeySequence> cycleForwardShortcuts() const;
    [[nodiscard]] QList<QKeySequence> cycleBackwardShortcuts() const;
    [[nodiscard]] QList<QKeySequence> activateSelectionAndHideShortcuts() const;
    [[nodiscard]] QList<QKeySequence> hideAppSwitcherShortcuts() const;

    [[nodiscard]] QMap<QPair<QString, ShortcutsFor>, QStringList>
        collectAllShortcuts() const;

    void manageShortcutsFor(ShortcutsFor whichShortcut,
                            ShortcutAction whichAction,
                            const QKeySequence& sequence);

  private:
    Settings();
    ~Settings() = default;

    [[nodiscard]] static QStringList qKeySequenceListToQStringList(
        const QList<QKeySequence>& qKeySequenceList);

    QSettings m_settings;
};

#endif  // SETTINGS_H
