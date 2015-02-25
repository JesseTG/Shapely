#ifndef SHAPELY_HPP
#define SHAPELY_HPP

#include <QMainWindow>
#include "model/ShapelyModel.hpp"

namespace Ui {
class Shapely;
}

template <class T> class QSharedPointer;

class ShapelyWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit ShapelyWindow(QWidget *parent = 0);
  QSharedPointer<ShapelyModel> currentModel() noexcept;
  ~ShapelyWindow();

private slots:
  void createPolygon() noexcept;

private:
  Ui::Shapely *ui;
  int _created;
};

#endif // SHAPELY_HPP
