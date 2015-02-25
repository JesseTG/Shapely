#include "ShapelyWidget.hpp"

#include <cstdlib>
#include <exception>
#include <limits>
#include <sstream>

#include <QDebug>
#include <QElapsedTimer>
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QPoint>
#include <QPolygonF>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QSharedPointer>
#include <QSurfaceFormat>
#include <QtMath>

#include "Constants.hpp"
#include "Utility.hpp"
#include "ShapelyWindow.hpp"
#include "renderer/AbstractRenderer.hpp"
#include "renderer/ShaderRenderer.hpp"
#include "renderer/MidpointRenderer.hpp"
#include "model/ShapelyModel.hpp"

constexpr QSurfaceFormat::FormatOption FORMAT_OPTION =
#ifdef DEBUG
    QSurfaceFormat::DebugContext;
#else
    QSurfaceFormat::StereoBuffers;
#endif
constexpr QSurfaceFormat::OpenGLContextProfile PROFILE =
    QSurfaceFormat::CoreProfile;
constexpr QSurfaceFormat::RenderableType RENDER_TYPE = QSurfaceFormat::OpenGL;
constexpr QSurfaceFormat::SwapBehavior SWAP_TYPE =
    QSurfaceFormat::DefaultSwapBehavior;
constexpr int SAMPLES = 0;

constexpr QSurfaceFormat::FormatOptions FLAGS(FORMAT_OPTION | PROFILE |
                                              RENDER_TYPE | SWAP_TYPE);

constexpr int NO_POINT_SELECTED = -1;
const QTransform REFLECT(0, -1, -1, 0, 0, 0);

constexpr QOpenGLBuffer::UsagePattern USAGE_PATTERN = QOpenGLBuffer::StaticDraw;

ShapelyWidget::ShapelyWidget(QWidget *parent)
    : QOpenGLWidget(parent), _selected(NO_POINT_SELECTED), _log(parent),
      _vbo(QOpenGLBuffer::VertexBuffer), _vao(),
      _default(Qt::CursorShape::ArrowCursor),
      _gripping(Qt::CursorShape::ClosedHandCursor) {
  QSurfaceFormat format(FLAGS);
  format.setSamples(SAMPLES);
  this->setFormat(format);
}

ShapelyWidget::~ShapelyWidget() {
  Q_ASSERT(this->_renderer);
  Q_ASSERT(this->_vbo.isCreated());
  Q_ASSERT(this->_vao.isCreated());
  this->_renderer.reset();
  _vbo.release();
  _vao.release();
  _vbo.destroy();
  _vao.destroy();
}

void ShapelyWidget::initializeGL() {
  using std::runtime_error;
  using std::ostringstream;
  this->initializeOpenGLFunctions();

  if (!_vao.create()) {
    ostringstream e;
    e << "Could not create VAO (ID: " << _vao.objectId() << ")";
    throw runtime_error(e.str());
  }
  _vao.bind();

  if (!_vbo.create()) {
    ostringstream e;
    e << "Could not create VBO (ID: " << _vbo.bufferId() << ")";
    throw runtime_error(e.str());
  }

  if (!_vbo.bind()) {
    ostringstream e;
    e << "Could not bind VBO (ID: " << _vbo.bufferId() << ")";
    throw runtime_error(e.str());
  }

  _vbo.setUsagePattern(USAGE_PATTERN);

  this->_renderer.reset(new ShaderRenderer(":/shader/shader.vert",
                                           ":/shader/shader.frag",
                                           this->context(), this, &this->_vbo));

  if (!_log.initialize()) {
    qWarning() << "GL_KHR_debug extension not supported, debug logging will "
                  "not be available\n";
  } else {
    _log.enableMessages();
    _log.startLogging();
  }

  QSharedPointer<ShapelyModel> model = this->_currentModel();
  if (model) {

    this->_renderer->updateData(*model);
    this->_renderer->updateView(*model);
  }
}

void ShapelyWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);

  QSharedPointer<ShapelyModel> model = this->_currentModel();
  if (model && model->polygon.size()) {
    Q_ASSERT(this->_renderer);
    Q_ASSERT(this->_vao.isCreated());
    Q_ASSERT(this->_vbo.isCreated());

    this->_renderer->drawPolygon();
  }

  if (this->_log.isLogging()) {
    for (const QOpenGLDebugMessage &message : this->_log.loggedMessages()) {
      qDebug() << message;
    }
  }
}

void ShapelyWidget::resizeGL(const int w, const int h) {
  QSharedPointer<ShapelyModel> model = this->_currentModel();
  this->_renderer->updateSize(w, h);
  this->update();
  if (model) {
    Q_ASSERT(this->_renderer);

    this->_renderer->updateView(*model);
  }
}

void ShapelyWidget::mouseMoveEvent(QMouseEvent *e) {
  if (this->_selected >= 0) {
    // If the use is dragging a vertex...
    QSharedPointer<ShapelyModel> model = this->_currentModel();

    if (model) {
      Q_ASSERT(this->_renderer);

      QPointF c(e->x(), e->y());
      c.setY(this->height() - c.y());
      c.setX(jtg::map<float>(c.x(), 0, this->width(), -1.0f, 1.0f));
      c.setY(jtg::map<float>(c.y(), 0, this->height(), -1.0f, 1.0f));
      QPointF coords = this->_renderer->unproject(c.x(), c.y());

      Q_ASSERT(0 <= this->_selected && this->_selected < model->polygon.size());
      model->polygon[this->_selected].setX(coords.x());
      model->polygon[this->_selected].setY(coords.y());

      this->_renderer->updateData(*model);
      this->update();
    }
  }
}

void ShapelyWidget::mouseDoubleClickEvent(QMouseEvent *) {}

void ShapelyWidget::mousePressEvent(QMouseEvent *e) {
  QSharedPointer<ShapelyModel> model = this->_currentModel();

  if (model) {
    Q_ASSERT(this->_renderer);

    QPointF c(e->x(), e->y());
    c.setY(this->height() - c.y());
    c.setX(jtg::map<float>(c.x(), 0, this->width(), -1.0f, 1.0f));
    c.setY(jtg::map<float>(c.y(), 0, this->height(), -1.0f, 1.0f));
    QPointF coords = this->_renderer->unproject(c.x(), c.y());
    int clicked = this->_clickedPoint(coords);

    if (e->button() == Qt::MouseButton::LeftButton) {
      // ^ this will be the last index when we add the next point

      if (clicked >= 0) {
        // If we clicked near an existing vertex...
        this->_selected = clicked;
        this->setCursor(this->_gripping);
#ifdef DEBUG
        qDebug() << "Selected vertex #" << this->_selected << "of"
                 << model->name;
#endif
      } else {
        this->_selected = model->polygon.size();
        model->polygon.append(coords);
#ifdef DEBUG
        qDebug() << "Adding a vertex to" << model->name;
#endif
      }

      this->_renderer->updateData(*model);
    } else if (e->button() == Qt::MouseButton::RightButton) {
      if (clicked >= 0) {
        Q_ASSERT(0 <= clicked && clicked < model->polygon.size());
        model->polygon.remove(clicked);

#ifdef DEBUG
        qDebug() << "Deleted vertex #" << clicked << "on" << model->name;
#endif
        this->_renderer->updateData(*model);
      }
    }

    this->_renderer->updateView(*model);
    this->update();
  }
}

void ShapelyWidget::mouseReleaseEvent(QMouseEvent *e) {

#ifdef DEBUG
  if (this->_selected != NO_POINT_SELECTED) {
    QSharedPointer<ShapelyModel> model = this->_currentModel();

    if (model) {
      qDebug() << "Vertex #" << this->_selected << "of" << model->name
               << " dropped at" << model->polygon.last() << "(mouse released at"
               << e->pos() << ")";
    }
  }
#endif
  this->_selected = NO_POINT_SELECTED;
  this->setCursor(this->_default);
  this->update();
}

// Part 5 of the assignment is implemented here!
void ShapelyWidget::translateX(const double x) {
  QSharedPointer<ShapelyModel> model = this->_currentModel();

  if (model) {
    Q_ASSERT(this->_renderer);

    float tx = model->transform.dx(); // horizontal translation
    model->transform.translate(x - tx, 0);

    this->_renderer->updateView(*model);

    this->update();
#ifdef DEBUG
    qDebug() << "Translated horizontally to" << x;
#endif
  }
}

void ShapelyWidget::translateY(const double y) {
  QSharedPointer<ShapelyModel> model = this->_currentModel();

  if (model) {
    Q_ASSERT(this->_renderer);

    float ty = model->transform.dy(); // vertical translation
    model->transform.translate(0, y - ty);

    this->_renderer->updateView(*model);

    this->update();
#ifdef DEBUG
    qDebug() << "Translated vertically to" << y;
#endif
  }
}

void ShapelyWidget::rotate(const int degrees) {
  QSharedPointer<ShapelyModel> model = this->_currentModel();
  if (model) {
    Q_ASSERT(this->_renderer);

    float d = degrees * (M_PI / 180.0f);
    float a = std::atan2(model->transform.m22(), model->transform.m21());
    model->transform.rotateRadians(-d - a);

    this->_renderer->updateView(*model);

    this->update();
#ifdef DEBUG
    qDebug() << "Rotated to" << degrees << "degrees";
#endif
  }
}

void ShapelyWidget::scaleX(double x) {
  x = std::round(x * 100) / 100;
  // Two digits of precision; mostly just to avoid numbers really close to
  // zero
  if (x) {
    QSharedPointer<ShapelyModel> model = this->_currentModel();

    if (model) {
      Q_ASSERT(this->_renderer);

      float sx = model->transform.m11(); // horizontal scaling factor
      model->transform.scale(x / sx, 1);
      this->_renderer->updateView(*model);

      this->update();
#ifdef DEBUG
      qDebug() << "Scaled horizontally to" << x;
#endif
    }
  }
}

void ShapelyWidget::scaleY(double y) {
  y = std::round(y * 100) / 100;
  if (y) {
    // IEEE 754 == multiple representations of 0 == ???
    QSharedPointer<ShapelyModel> model = this->_currentModel();

    if (model) {
      Q_ASSERT(this->_renderer);

      float sy = model->transform.m22(); // vertical scaling factor
      model->transform.scale(1, y / sy);
      this->_renderer->updateView(*model);

      this->update();
#ifdef DEBUG
      qDebug() << "Scaled vertically to" << y;
#endif
    }
  }
}

void ShapelyWidget::shearX(double x) {
  x = std::round(x * 100) / 100;
  if (x) {
    QSharedPointer<ShapelyModel> model = this->_currentModel();
    if (model) {
      Q_ASSERT(this->_renderer);

      float sx = model->transform.m21(); // horizontal shearing
      model->transform.shear(x - sx, 0);
      this->_renderer->updateView(*model);

#ifdef DEBUG
      qDebug() << "Sheared horizontally to" << x;
#endif
    }
    this->update();
  }
}

void ShapelyWidget::shearY(double y) {
  y = std::round(y * 100) / 100;
  if (y) {
    QSharedPointer<ShapelyModel> model = this->_currentModel();
    if (model) {

      Q_ASSERT(this->_renderer);

      float sy = model->transform.m12(); // horizontal shearing
      model->transform.shear(0, y - sy);
      this->_renderer->updateView(*model);

#ifdef DEBUG
      qDebug() << "Sheared vertically to" << y;
#endif
    }
    this->update();
  }
}

void ShapelyWidget::reflect() {
  QSharedPointer<ShapelyModel> model = this->_currentModel();

  if (model) {
    Q_ASSERT(this->_renderer);

    model->transform *= REFLECT;
    this->_renderer->updateView(*model);
  }
  this->update();
}

void ShapelyWidget::setRenderer(const int hardware) {
  if (hardware) {
    this->_renderer.reset(
        new ShaderRenderer(":/shader/shader.vert", ":/shader/shader.frag",
                           this->context(), this, &this->_vbo));
  } else {
    this->_renderer.reset(
        new MidpointRenderer(":/shader/shader.vert", ":/shader/shader.frag",
                             this->context(), this, &this->_vbo));
  }

#ifdef DEBUG
  qDebug() << ((hardware) ? "Enabled" : "Disabled") << "hardware rasterization";
#endif

  QSharedPointer<ShapelyModel> model = this->_currentModel();

  if (model) {
    Q_ASSERT(this->_renderer);

    this->_renderer->updateSize(this->width(), this->height());
    this->_renderer->updateData(*model);
    this->_renderer->updateView(*model);
  }
  this->update();
}

void ShapelyWidget::setModel(QListWidgetItem *current, QListWidgetItem *prev) {
  Q_ASSERT(this->_renderer);
  constexpr int ROLE = Constants::MODEL_ROLE;

  typedef QSharedPointer<ShapelyModel> ModelPtr;
  ModelPtr now = current ? current->data(ROLE).value<ModelPtr>() : ModelPtr();
#ifdef DEBUG
  ModelPtr old = prev ? prev->data(ROLE).value<ModelPtr>() : ModelPtr();

  if (old && now) {
    qDebug() << "Switching from model" << old->name << "to" << now->name;
  } else if (now && !old) {
    qDebug() << "Selecting model" << now->name;
  } else if (!now && old) {
    qDebug() << "Deselected " << old->name;
  } else {
    qDebug() << "No model selected";
  }
#endif

  if (now) {
    this->_renderer->updateData(*now);
    this->_renderer->updateView(*now);
  }
  this->update();
}

QSharedPointer<ShapelyModel> ShapelyWidget::_currentModel() noexcept {
  ShapelyWindow *w = static_cast<ShapelyWindow *>(this->window());
  Q_ASSERT(typeid(*w) == typeid(ShapelyWindow));
  return w->currentModel();
}

int ShapelyWidget::_clickedPoint(const QPointF &point) noexcept {
  QSharedPointer<ShapelyModel> model = this->_currentModel();

  if (model) {
    for (int i = 0; i < model->polygon.size(); ++i) {
      const QPointF &p = model->polygon[i];
      QPointF delta = p - point;
      if (delta.x() * delta.x() + delta.y() * delta.y() <=
          Constants::MARKER_RADIUS * Constants::MARKER_RADIUS) {
        return i;
      }
    }
  }

  return NO_POINT_SELECTED;
}
