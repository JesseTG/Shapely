#ifndef MIDPOINTRENDERER_HPP
#define MIDPOINTRENDERER_HPP

#include <map>
#include <vector>

#include "AbstractRenderer.hpp"

#include "model/ShapelyModel.hpp"

class QOpenGLContext;
class QOpenGLFunctions;
class QPoint;

class MidpointRenderer : public AbstractRenderer {
public:
  MidpointRenderer(const QString &vertPath, const QString &fragPath,
                   QOpenGLContext *context, QOpenGLFunctions *gl,
                   QOpenGLBuffer *vbo);
  virtual ~MidpointRenderer();
  virtual void drawPolygon() override;
  virtual void updateData(const ShapelyModel &) override;
  virtual void updateView(const ShapelyModel &) override;

protected:
  virtual void drawBackground() override;
  virtual void drawLines() override;
  virtual void fillPolygon() override;

private:
  void _computeLine(QPoint, QPoint) noexcept;
  void _fill() noexcept;

  ShapelyModel _current;

  typedef GLfloat CoordType;
  // ^ So if I change the data representation this changes, too

  std::vector<CoordType> _linePixels;
  std::vector<CoordType> _fillPixels;

  std::map<int, std::vector<int>> _intersections;
  // y, [x, x, x, x...]
  int _position;
  int _matrix;
  int _color;

  int _lineNumsOffset;
  int _fillNumsOffset;
};

#endif // MIDPOINTRENDERER_HPP
