#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <qnamespace.h>

namespace Constants {
constexpr int SELECTION_COMMIT_TIMEOUT_MS = 1000;

constexpr auto INTERNAL_LIST_DATA_ROLE = Qt::UserRole + 1;
constexpr auto SLOT_INDEX_ROLE = Qt::UserRole + 2;
}  // namespace Constants

#endif  // CONSTANTS_H
