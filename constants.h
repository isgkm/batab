#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QObject>

namespace Constants {
inline constexpr auto ROLE_INTERNAL_LIST_DATA = Qt::UserRole + 1;
inline constexpr auto ROLE_SLOT_INDEX = Qt::UserRole + 2;

inline constexpr auto SETTINGS_KEY_QUICK_SWITCH_HOLD_THRESHOLD_MS =
    QLatin1StringView("quickSwitch/holdThresholdMs");

inline constexpr auto SETTINGS_KEY_SELECTION_COMMIT_TIMEOUT_MS =
    QLatin1StringView("selection/commitTimeoutMs");

inline constexpr auto SETTINGS_KEY_HOT_CORNER_SIZE =
    QLatin1StringView("hotCorner/size");

inline constexpr auto SETTINGS_KEY_HOT_CORNER_ELAPSED_TIME_MS =
    QLatin1StringView("hotCorner/elapsedTimeMs");

}  // namespace Constants

#endif  // CONSTANTS_H
