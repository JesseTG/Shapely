#ifndef ABSTRACTRENDERER_HPP
#define ABSTRACTRENDERER_HPP

#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>
#include <QSize>

class QOpenGLContext;
class QOpenGLFunctions;
class QOpenGLBuffer;
struct ShapelyModel;

class AbstractRenderer {
public:
  AbstractRenderer(QOpenGLContext *, QOpenGLFunctions *, QOpenGLBuffer *);
  virtual ~AbstractRenderer();
  virtual void drawPolygon() = 0;
  virtual void updateData(const ShapelyModel &) = 0;
  virtual void updateView(const ShapelyModel &) = 0;
  QPointF unproject(const float x, const float y) const noexcept;
  QPointF project(const float x, const float y) const noexcept;
  void updateSize(const int w, const int h);

protected:
  virtual void drawBackground() = 0;
  virtual void drawLines() = 0;
  virtual void fillPolygon() = 0;

  QOpenGLFunctions *gl;
  QOpenGLContext *context;
  QOpenGLBuffer *vbo;
  QOpenGLShader vert;
  QOpenGLShader frag;
  QOpenGLShaderProgram shader;

  QMatrix4x4 projection;
  QMatrix4x4 view;
  QMatrix4x4 worldToScreen;
  QMatrix4x4 screenToWorld;
  QSize size;

  bool dataChanged : 1;
  bool viewChanged : 1;
  bool shouldFillPolygon : 1;
};

#endif // ABSTRACTRENDERER_HPP
