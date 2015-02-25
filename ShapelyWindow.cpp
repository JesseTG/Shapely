#include "ShapelyWindow.hpp"
#include "ui_shapely.h"

#include <QListWidgetItem>
#include <QPointF>
#include <QSharedPointer>
#include <QVariant>

#include "Constants.hpp"
#include "model/ShapelyModel.hpp"

ShapelyWindow::ShapelyWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::Shapely), _created(0) {
  ui->setupUi(this);
}

ShapelyWindow::~ShapelyWindow() { delete ui; }

QSharedPointer<ShapelyModel> ShapelyWindow::currentModel() noexcept {
  QListWidgetItem *item = this->ui->polygons->currentItem();
  if (item) {
    return item->data(Constants::MODEL_ROLE)
        .value<QSharedPointer<ShapelyModel>>();
  } else {
    return QSharedPointer<ShapelyModel>(nullptr);
  }
}

void ShapelyWindow::createPolygon() noexcept {
  QString name = QString("polygon%1").arg(QString::number(this->_created++));
  QListWidgetItem *item =
      new QListWidgetItem(name, nullptr, QListWidgetItem::UserType);
  QSharedPointer<ShapelyModel> model = QSharedPointer<ShapelyModel>::create();
  model->name = name;

  QVariant data = QVariant::fromValue(model);

  item->setData(Constants::MODEL_ROLE, data);
  this->ui->polygons->addItem(item);
#ifdef DEBUG
  qDebug() << "Created a new polygon named"
           << data.value<QSharedPointer<ShapelyModel>>()->name;
#endif
  this->ui->polygons->setCurrentItem(item);
}
