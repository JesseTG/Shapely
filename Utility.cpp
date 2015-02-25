#include "Utility.hpp"

#include <QtGlobal>
#include <QLineF>
#include <QPair>
#include <QPoint>
#include <QPolygonF>
#include <QVector>

namespace jtg {

bool isSimplePolygon(const QPolygonF &polygon) noexcept {
  if (polygon.size() < 3)
    return false;

  if (polygon.size() == 3)
    return true;

  for (int i = 0; i < polygon.size(); ++i) {
    if (i < polygon.size() - 1) {
      // When we reach the end...
      for (int h = i + 1; h < polygon.size(); ++h) {

        if (polygon[i] == polygon[h]) {
          // If we have a duplicate vertex...

          return false;
        }
      }
    }

    int j = (i + 1) % polygon.size();
    QPointF iToj = polygon[j] - polygon[i];
    QPointF iTojNormal(iToj.y(), -iToj.x());
    // i is the first vertex and j is the second

    int startK = (j + 1) % polygon.size();
    int endK = (i - 1 + polygon.size()) % polygon.size();
    endK += startK < endK ? 0 : startK + 1;
    int k = startK;

    QPointF iTok = polygon[k] - polygon[i];

    bool onLeftSide = QPointF::dotProduct(iTok, iTojNormal) >= 0;

    QPointF prevK = polygon[k];

    ++k;

    for (; k <= endK; ++k) {
      int modK = k % polygon.size();

      iTok = polygon[modK] - polygon[i];

      if (onLeftSide != QPointF::dotProduct(iTok, iTojNormal) >= 0) {

        QPointF prevKtoK = polygon[modK] - prevK;

        QPointF prevKtoKNormal(prevKtoK.y(), -prevKtoK.x());

        if ((QPointF::dotProduct(polygon[i] - prevK, prevKtoKNormal) >= 0) !=
            (QPointF::dotProduct(polygon[j] - prevK, prevKtoKNormal) >= 0)) {
          return false;
        }
      }

      onLeftSide = QPointF::dotProduct(iTok, iTojNormal) > 0;

      prevK = polygon[modK];
    }
  }

  return true;
}

/**
 * @brief decomposePolygon
 * @param polygon The polygon to decompose
 * @return A QPair containing a vertex buffer and an index buffer
 */
QPair<QVector<float>, QVector<uint16_t>>
decomposePolygon(const QPolygonF &polygon) noexcept {
  return QPair<QVector<float>, QVector<uint16_t>>();
}
}
