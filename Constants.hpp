#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <Qt>

class QColor;

namespace Constants {
constexpr int PIXELS_PER_WORLD = 64;
constexpr float WORLD_PER_PIXEL = 1.0f / PIXELS_PER_WORLD;
constexpr int MARKER_RADIUS = 32;     // In pixels
constexpr int MARKER_RESOLUTION = 18; // Number of vertices

constexpr int MODEL_ROLE = Qt::ItemDataRole::UserRole;

extern const QColor BACKGROUND_COLOR;
extern const QColor OUTLINE_COLOR;
extern const QColor POLYGON_COLOR;
extern const QColor COMPLEX_OUTLINE;
}
#endif // CONSTANTS_HPP
