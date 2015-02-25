#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <cstdint>

class QPoint;
class QPolygonF;
template <class T, class U> struct QPair;
template <class T> class QVector;

namespace jtg {
using std::uint16_t;

template <class T>
T map(T x, T in_min, T in_max, T out_min, T out_max) noexcept {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template <class T> T sign(T in) noexcept {
  if (in > 0)
    return 1;
  else if (in < 0)
    return -1;
  else
    return 0;
}

/**
 * @brief isSimplePolygon
 * @param polygon
 * @return True if the polygon does not self-intersect, false if it does or has
 * less than 3 vertices
 */
bool isSimplePolygon(const QPolygonF &polygon) noexcept;

QPair<QVector<float>, QVector<uint16_t>>
decomposePolygon(const QPolygonF &polygon) noexcept;
}

#endif // UTILITY_HPP
