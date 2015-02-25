#ifndef SHAPELYWIDGET_HPP
#define SHAPELYWIDGET_HPP

#include <memory>

#include <QCursor>
#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector>

#include "renderer/AbstractRenderer.hpp"

struct ShapelyModel;
class QMouseEvent;
class QPoint;
class ShapelyWindow;
class QListWidgetItem;
template <class T> class QSharedPointer;

class ShapelyWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  ShapelyWidget(QWidget *parent);
  virtual ~ShapelyWidget();

protected:
  void paintGL() override;
  void resizeGL(const int w, const int h) override;
  void initializeGL() override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseDoubleClickEvent(QMouseEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

signals:
  void finishedRendering(int ms);

private:
  QCursor _default;
  QCursor _gripping;
  QSharedPointer<ShapelyModel> _currentModel() noexcept;
  int _clickedPoint(const QPointF &point) noexcept;

  std::unique_ptr<AbstractRenderer> _renderer;
  int _selected;

  QOpenGLDebugLogger _log;
  QOpenGLBuffer _vbo;
  QOpenGLVertexArrayObject _vao;

private slots:
  // Transformations ///////////////////////////////////////////////////////////
  void translateX(const double);
  void translateY(const double);
  void rotate(const int);
  void scaleX(const double);
  void scaleY(const double);
  void shearX(const double);
  void shearY(const double);
  void reflect();
  //////////////////////////////////////////////////////////////////////////////

  void setRenderer(const int);
  void setModel(QListWidgetItem *current, QListWidgetItem *prev);
};

#endif // SHAPELYWIDGET_HPP
