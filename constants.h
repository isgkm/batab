#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "hotcornertriggerarea.h"

#include <QObject>

namespace Constants {
inline constexpr auto BATAB_VERSION = QLatin1StringView("0.0.1");

inline constexpr auto ROLE_INTERNAL_LIST_DATA = Qt::UserRole + 1;
inline constexpr auto ROLE_SLOT_INDEX = Qt::UserRole + 2;
inline constexpr auto ROLE_SHORTCUT_FOR_ACTION = Qt::UserRole + 3;

inline constexpr auto DEFAULT_VALUE_LAUNCH_ON_SYSTEM_STARTUP = false;
inline constexpr auto SETTINGS_KEY_LAUNCH_ON_SYSTEM_STARTUP =
    QLatin1StringView("general/launchOnSystemStartup");

inline constexpr auto DEFAULT_VALUE_QUICK_SWITCH_HOLD_THRESHOLD_MS = 100;
inline constexpr auto SETTINGS_KEY_QUICK_SWITCH_HOLD_THRESHOLD_MS =
    QLatin1StringView("general/holdThresholdMs");

inline constexpr auto DEFAULT_VALUE_SELECTION_COMMIT_TIMEOUT_MS = 1000;
inline constexpr auto SETTINGS_KEY_SELECTION_COMMIT_TIMEOUT_MS =
    QLatin1StringView("general/commitTimeoutMs");

inline constexpr auto DEFAULT_VALUE_HOT_CORNER_ENABLED = false;
inline constexpr auto SETTINGS_KEY_HOT_CORNER_ENABLED =
    QLatin1StringView("hotCorner/enabled");

inline constexpr auto DEFAULT_VALUE_HOT_CORNER_TRIGGER_AREA =
    HotCornerTriggerArea::TopLeft;
inline constexpr auto SETTINGS_KEY_HOT_CORNER_TRIGGER_AREA =
    QLatin1StringView("hotCorner/triggerArea");

inline constexpr auto DEFAULT_VALUE_HOT_CORNER_SIZE = 32;
inline constexpr auto SETTINGS_KEY_HOT_CORNER_SIZE =
    QLatin1StringView("hotCorner/size");

inline constexpr auto DEFAULT_VALUE_HOT_CORNER_REQUIRED_TIME_MS = 1000;
inline constexpr auto SETTINGS_KEY_HOT_CORNER_REQUIRED_TIME_MS =
    QLatin1StringView("hotCorner/requiredTimeMs");

inline constexpr auto REGKEY_WINDOWS_AUTOSTART = QLatin1StringView(
    R"regkey(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)regkey");

// NOLINTNEXTLINE(bugprone-throwing-static-initialization, cert-err58-cpp)
inline const QStringList DEFAULT_SHORTCUTS_CYCLE_FORWARD = {
    QStringLiteral("Tab"), QStringLiteral("Ctrl+N")};
inline constexpr auto SETTINGS_KEY_CYCLE_FORWARD_SHORTCUTS =
    QLatin1StringView("shortcuts/cycleForward");

// NOLINTNEXTLINE(bugprone-throwing-static-initialization, cert-err58-cpp)
inline const QStringList DEFAULT_SHORTCUTS_CYCLE_BACKWARD = {
    QStringLiteral("Shift+Tab"), QStringLiteral("Ctrl+P")};
inline constexpr auto SETTINGS_KEY_CYCLE_BACKWARD_SHORTCUTS =
    QLatin1StringView("shortcuts/cycleBackward");

// NOLINTNEXTLINE(bugprone-throwing-static-initialization, cert-err58-cpp)
inline const QStringList DEFAULT_SHORTCUTS_ACTIVATE_SELECTION_AND_HIDE = {
    QStringLiteral("Enter"), QStringLiteral("Return"),
    QStringLiteral("Ctrl+Y")};
inline constexpr auto SETTINGS_KEY_ACTIVATE_SELECTION_AND_HIDE =
    QLatin1StringView("shortcuts/activateSelectionAndHide");

// NOLINTNEXTLINE(bugprone-throwing-static-initialization, cert-err58-cpp)
inline const QStringList DEFAULT_SHORTCUTS_HIDE_APP_SWITCHER = {
    QStringLiteral("Escape"), QStringLiteral("Ctrl+Q")};
inline constexpr auto SETTINGS_KEY_HIDE_APP_SWITCHER =
    QLatin1StringView("shortcuts/hideAppSwitcher");

}  // namespace Constants

#endif  // CONSTANTS_H
