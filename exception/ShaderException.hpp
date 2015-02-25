#ifndef SHADEREXCEPTION_HPP
#define SHADEREXCEPTION_HPP

#include <stdexcept>
#include <string>

class QOpenGLShader;

class ShaderException : public std::runtime_error
{
public:
    ShaderException(const QOpenGLShader&) noexcept;
    const char* what() const noexcept override;
private:
    std::string _error;
};

#endif // SHADEREXCEPTION_HPP
