#ifndef SHADERRENDERER_HPP
#define SHADERRENDERER_HPP

#include <vector>

#include "AbstractRenderer.hpp"

class QOpenGLContext;
class QOpenGLFunctions;
class QOpenGLShader;
class QOpenGLShaderProgram;
struct ShapelyModel;

class ShaderRenderer : public AbstractRenderer {
public:
  ShaderRenderer(const QString &, const QString &, QOpenGLContext *,
                 QOpenGLFunctions *, QOpenGLBuffer *vbo);
  virtual ~ShaderRenderer();
  virtual void drawPolygon() override;
  virtual void updateData(const ShapelyModel &) override;
  virtual void updateView(const ShapelyModel &) override;

protected:
  virtual void drawBackground() override;
  virtual void drawLines() override;
  virtual void fillPolygon() override;

private:
  typedef GLfloat NumberType;
  std::vector<NumberType> _vertices;

  int _vertexOffset;
  int _markerOffset;
  int _position;
  int _matrix;
  int _color;
};

#endif // SHADERRENDERER_HPP
