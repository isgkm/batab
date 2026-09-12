#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <qnamespace.h>

namespace Constants {
constexpr int SELECTION_COMMIT_TIMEOUT_MS = 1000;

constexpr auto ROLE_INTERNAL_LIST_DATA = Qt::UserRole + 1;
constexpr auto ROLE_SLOT_INDEX = Qt::UserRole + 2;
}  // namespace Constants

#endif  // CONSTANTS_H
