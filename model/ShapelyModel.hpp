#ifndef SHAPELYMODEL_HPP
#define SHAPELYMODEL_HPP

#include <QPointF>
#include <QPolygonF>
#include <QSharedPointer>
#include <QString>
#include <QTransform>

struct ShapelyModel {
  QPolygonF polygon;
  QPointF cameraCoords;
  QTransform transform;
  QString name;
  float zoom;
};

Q_DECLARE_METATYPE(QSharedPointer<ShapelyModel>)
// ^ Need this to have ShapelyModels in the QListWidget

#endif // SHAPELYMODEL_HPP
