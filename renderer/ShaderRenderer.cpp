#include "ShaderRenderer.hpp"

#include <string>

#include <QtGlobal>
#include <QColor>
#include <QVector4D>
#include <QMatrix3x3>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QSurfaceFormat>
#include <QTransform>

#include "Constants.hpp"
#include "Utility.hpp"
#include "exception/ShaderException.hpp"
#include "exception/ShaderProgramException.hpp"
#include "model/ShapelyModel.hpp"

ShaderRenderer::ShaderRenderer(const QString &vertPath, const QString &fragPath,
                               QOpenGLContext *context, QOpenGLFunctions *gl,
                               QOpenGLBuffer *vbo)
    : AbstractRenderer(context, gl, vbo) {

  if (!this->vert.compileSourceFile(vertPath)) {
    throw ShaderException(vert);
  }

  if (!frag.compileSourceFile(fragPath)) {
    throw ShaderException(frag);
  }

  if (!(shader.addShader(&vert) && shader.addShader(&frag))) {
    // If the vertex and fragment shaders were not properly added...
    throw ShaderProgramException(shader);
  }

  if (!(shader.link() && shader.bind())) {
    throw ShaderProgramException(shader);
  }

  this->_matrix = shader.uniformLocation("matrix");
  this->_color = shader.uniformLocation("color");

  this->_position = shader.attributeLocation("position");
  shader.enableAttributeArray(this->_position);
  this->gl->glVertexAttribPointer(this->_position, 2, GL_FLOAT, GL_TRUE, 0, 0);
  shader.setUniformValue(this->_matrix, QMatrix4x4(this->worldToScreen));
}

ShaderRenderer::~ShaderRenderer() {
  shader.disableAttributeArray(this->_position);
}

void ShaderRenderer::updateData(const ShapelyModel &model) {
  using std::sin;
  using std::cos;
  using Constants::MARKER_RADIUS;
  using Constants::MARKER_RESOLUTION;
  this->_vertices.clear();
  this->_vertices.reserve(model.polygon.size() * 2);
  // may change if more vertex attributes are added

  for (const QPointF &point : model.polygon) {
    this->_vertices.push_back(point.x());
    this->_vertices.push_back(point.y());
  }

  this->shouldFillPolygon = jtg::isSimplePolygon(model.polygon);
  this->dataChanged = true;
}

void ShaderRenderer::updateView(const ShapelyModel &model) {
  float w = this->size.width();
  float h = this->size.height();

  if (!model.transform.isInvertible()) {
    qWarning() << "Warning: Non-invertible model transform" << model.transform;
  }

  QMatrix4x4 p;
  p.ortho(-w, w, -h, h, 0, 1);

  QMatrix4x4 v;
  v.translate(model.cameraCoords.x(), model.cameraCoords.y());

  QMatrix4x4 wTs = p * v * model.transform;
  QMatrix4x4 sTw = wTs.inverted();
  if (!sTw.isIdentity()) {
    // If this transform is invertible...
    this->view = v;
    this->worldToScreen = wTs;
    this->screenToWorld = sTw;
    this->projection = p;
    this->viewChanged = true;
  }
}

void ShaderRenderer::drawBackground() {}

void ShaderRenderer::drawLines() {
  shader.setUniformValue(this->_color, this->shouldFillPolygon
                                           ? Constants::OUTLINE_COLOR
                                           : Constants::COMPLEX_OUTLINE);
  this->gl->glDrawArrays(GL_LINE_LOOP, this->_vertexOffset,
                         this->_vertices.size() / 2); // hack
}

void ShaderRenderer::fillPolygon() {
  shader.setUniformValue(this->_color, Constants::POLYGON_COLOR);
  this->gl->glDrawArrays(GL_TRIANGLE_FAN, this->_vertexOffset,
                         this->_vertices.size() / 2); // hack
}

void ShaderRenderer::drawPolygon() {
  Q_ASSERT(shader.isLinked());
  Q_ASSERT(vert.isCompiled() && frag.isCompiled());

  if (this->dataChanged) {
    this->_vertexOffset = 0;
    vbo->allocate(this->_vertices.data(),
                  this->_vertices.size() * sizeof(NumberType));
    this->dataChanged = false;
  }

  if (this->viewChanged) {
    shader.setUniformValue(this->_matrix, this->worldToScreen);
    this->viewChanged = false;
  }

  if (this->shouldFillPolygon) {
    this->fillPolygon();
  }

  this->drawLines();
}
