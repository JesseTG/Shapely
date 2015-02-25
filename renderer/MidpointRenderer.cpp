#include "MidpointRenderer.hpp"

#include <algorithm>
#include <cmath>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QPointF>
#include <QPoint>
#include <QtMath>

#include "exception/ShaderException.hpp"
#include "exception/ShaderProgramException.hpp"
#include "Constants.hpp"
#include "Utility.hpp"

constexpr QOpenGLBuffer::UsagePattern USAGE_PATTERN = QOpenGLBuffer::StaticDraw;

// TODO: Only drawing a dot at the center; presumably -1 to 1; gotta map these
// back to screen dimensions

// Make the renderers responsible for projection
MidpointRenderer::MidpointRenderer(const QString &vertPath,
                                   const QString &fragPath,
                                   QOpenGLContext *context,
                                   QOpenGLFunctions *gl, QOpenGLBuffer *vbo)
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
  this->gl->glVertexAttribPointer(this->_position, 2, GL_FLOAT, GL_FALSE, 0, 0);
  shader.setUniformValue(this->_matrix, this->worldToScreen);
}

MidpointRenderer::~MidpointRenderer() {
  shader.disableAttributeArray(this->_position);
}

void MidpointRenderer::drawBackground() {}

void MidpointRenderer::updateData(const ShapelyModel &model) {
  this->_current = model;
  this->updateView(model);
  this->dataChanged = true;
}

void MidpointRenderer::updateView(const ShapelyModel &model) {
  using std::round;
  using std::abs;

  if (this->dataChanged)
    return;
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

    this->_linePixels.clear();

    const QPolygonF &polygon = this->_current.polygon;
    int verts = polygon.size();

    QRectF rect = model.polygon.boundingRect();
    float area = rect.width() * rect.height();
    // Rough area approximation so we can preallocate memory

    this->_intersections.clear();
    this->_linePixels.reserve(area * 2);
    for (int i = 0; i < verts; ++i) {
      const QPointF &a = polygon[i];
      const QPointF &b = polygon[(i + 1) % verts];

      this->_computeLine(QPoint(round(a.x()), round(a.y())),
                         QPoint(round(b.x()), round(b.y())));
    }

    // NOTE: Can optimize; only need to check new edges for intersection
    if (jtg::isSimplePolygon(polygon)) {
      // If the polygon self-intersects or doesn't have 3 sides...
      this->shouldFillPolygon = true;
      this->_fillPixels.clear();

      this->_fillPixels.reserve(area * 2);
      this->_fill();
    } else {
      this->shouldFillPolygon = false;
    }
    this->viewChanged = true;
  }
}

void MidpointRenderer::drawLines() {
  shader.setUniformValue(this->_color, this->shouldFillPolygon
                                           ? Constants::OUTLINE_COLOR
                                           : Constants::COMPLEX_OUTLINE);
  this->gl->glDrawArrays(GL_POINTS, this->_lineNumsOffset,
                         this->_linePixels.size() / 2); // hack
}

void MidpointRenderer::fillPolygon() {
  shader.setUniformValue(this->_color, Constants::POLYGON_COLOR);
  this->gl->glDrawArrays(GL_POINTS, this->_fillNumsOffset,
                         this->_fillPixels.size() / 2); // hack
}

void MidpointRenderer::drawPolygon() {
  Q_ASSERT(shader.isLinked());
  Q_ASSERT(vert.isCompiled() && frag.isCompiled());

  if (this->dataChanged || this->viewChanged) {
    int lineNums = this->_linePixels.size();
    int fillNums = (this->shouldFillPolygon) ? this->_fillPixels.size() : 0;

    this->_lineNumsOffset = 0;
    this->_fillNumsOffset =
        (this->_lineNumsOffset + lineNums) * sizeof(CoordType);

    vbo->allocate((lineNums + fillNums) * sizeof(CoordType));

    // The VBO will contain line coordinates, vertex marker coordinates, and
    // fill coordinates (if applicable), in that order
    vbo->write(this->_lineNumsOffset, this->_linePixels.data(),
               lineNums * sizeof(CoordType));
    if (fillNums) {
      // If we're filling the polygon, then write the coordinates in
      vbo->write(this->_fillNumsOffset, this->_fillPixels.data(),
                 fillNums * sizeof(CoordType));
    }

    shader.setUniformValue(this->_matrix, this->worldToScreen);

    this->dataChanged = false;
    this->viewChanged = false;
  }

  if (this->shouldFillPolygon) {
    this->fillPolygon();
  }

  this->drawLines();
}

/**
 * Given two points, add the coordinates of each pixel between them to the
 * current buffer of line pixels
 */
void MidpointRenderer::_computeLine(QPoint a, QPoint b) noexcept {
  using jtg::sign;
  using std::abs;

  int width = b.x() - a.x();
  int height = b.y() - a.y();

  // In any Bresenham run, there's always two directions you can go; east or
  // northeast.  This screwing around with signs below helps generalize that to
  // other slopes (e.g. east/south-east for -1 < slope < 0)
  int dx = sign(width);
  int dy = sign(height);

  int du = dx;
  int dv = dy;

  int longest = abs(width);
  int shortest = abs(height);

  if (longest <= shortest) {
    // If the line's absolute slope is greater than 1...
    longest = abs(height);
    shortest = abs(width);
    // ...then swap the axes

    du = 0;
    dv = dy;
  } else {
    du = dx;
    dv = 0;
  }

  int rise = longest / 2;
  // integer multiplies/divides by powers of two usually compile to bit-shifts

  for (int i = 0; i < longest; ++i) {
    // For each vertical or horizontal (whichever is longer) pixel spanned...
    this->_linePixels.push_back(a.x());
    this->_linePixels.push_back(a.y());
    // Queue up these pixels for rendering

    std::vector<int> &row = this->_intersections[a.y()];

    if (row.empty() || row.back() != a.x()) {
      row.push_back(a.x());
    }
    // Kill two birds with one stone; we know the scan lines intersect here, so
    // why not get them ready for rendering?

    rise += shortest;
    if (rise > longest) {
      rise -= longest;
      a += {dx, dy};
    } else {
      a += {du, dv};
    }
  }
}

void MidpointRenderer::_fill() noexcept {

  for (auto &i : this->_intersections) {
    int y = i.first;
    auto &second = i.second;
    int size = second.size();
    std::sort(second.begin(), second.end());
    for (int j = 0; j < size - 1; j += 2) {
      int x1 = second[j + 1];

      for (int x0 = second[j]; x0 < x1; ++x0) {
        this->_fillPixels.push_back(x0);
        this->_fillPixels.push_back(y);
      }
    }
  }
}
