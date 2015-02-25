#include "AbstractRenderer.hpp"

#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

AbstractRenderer::AbstractRenderer(QOpenGLContext *context,
                                   QOpenGLFunctions *gl, QOpenGLBuffer *vbo)
    : gl(gl), context(context), vert(QOpenGLShader::Vertex),
      frag(QOpenGLShader::Fragment), shader(context), vbo(vbo),
      dataChanged(false), viewChanged(false), shouldFillPolygon(false) {}

AbstractRenderer::~AbstractRenderer() {
  Q_ASSERT(shader.isLinked());

  shader.release();
  shader.removeAllShaders();

  Q_ASSERT(shader.shaders().size() == 0);
}

QPointF AbstractRenderer::unproject(const float x, const float y) const
    noexcept {
  Q_ASSERT(qIsFinite(x) && qIsFinite(y));
  return this->screenToWorld.map(QPointF(x, y));
}

QPointF AbstractRenderer::project(const float x, const float y) const noexcept {
  Q_ASSERT(qIsFinite(x) && qIsFinite(y));
  return this->worldToScreen.map(QPointF(x, y));
}

void AbstractRenderer::updateSize(const int w, const int h) {
  this->size.setWidth(w);
  this->size.setHeight(h);
}
