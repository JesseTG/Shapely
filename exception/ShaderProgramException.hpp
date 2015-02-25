#ifndef SHADERPROGRAMEXCEPTION_HPP
#define SHADERPROGRAMEXCEPTION_HPP

#include <stdexcept>
#include <string>

class QOpenGLShaderProgram;

class ShaderProgramException : public std::runtime_error
{
public:
    ShaderProgramException(const QOpenGLShaderProgram&);
    const char* what() const noexcept override;
private:
    std::string _error;
};

#endif // SHADERPROGRAMEXCEPTION_HPP
