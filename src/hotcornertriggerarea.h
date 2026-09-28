#ifndef HOTCORNERTRIGGERAREA_H
#define HOTCORNERTRIGGERAREA_H

#include <cstdint>

enum class HotCornerTriggerArea : std::uint8_t {
    TopLeft,
    Top,
    TopRight,
    Right,
    BottomRight,
    Bottom,
    BottomLeft,
    Left
};

#endif  // HOTCORNERTRIGGERAREA_H
