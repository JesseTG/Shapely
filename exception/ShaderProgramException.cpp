#include "ShaderProgramException.hpp"

#include <sstream>

#include <QOpenGLShaderProgram>

ShaderProgramException::ShaderProgramException(const QOpenGLShaderProgram &shader) : std::runtime_error("") {
  std::ostringstream e;

  e << "Shader program (ID " << shader.programId() << ") error:\n\t" << shader.log().toStdString();
  this->_error = e.str();
}

const char* ShaderProgramException::what() const noexcept {
    return this->_error.c_str();
}
