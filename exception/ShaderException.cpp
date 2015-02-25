#include "ShaderException.hpp"

#include <sstream>

#include <QOpenGLShader>

ShaderException::ShaderException(const QOpenGLShader &shader) noexcept : std::runtime_error("") {
  std::ostringstream e;
  switch (shader.shaderType()) {
  case QOpenGLShader::Vertex:
    e << "Vertex";
    break;
  case QOpenGLShader::Fragment:
    e << "Fragment";
    break;
  case QOpenGLShader::Geometry:
    e << "Geometry";
    break;
  case QOpenGLShader::TessellationControl:
    e << "Tesselation control";
    break;
  case QOpenGLShader::TessellationEvaluation:
    e << "Tesselation evaluation";
    break;
  case QOpenGLShader::Compute:
    e << "Compute";
    break;
  default:
    e << "Invalid";
  }
  e << " shader (ID " << shader.shaderId() << ") error:\n\t" << shader.log().toStdString();
  this->_error = e.str();
}

const char* ShaderException::what() const noexcept {
    return this->_error.c_str();
}
