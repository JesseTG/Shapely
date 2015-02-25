#version 130
uniform mat4 matrix;
uniform vec4 color;

attribute vec2 position;

void main(void)
{
    gl_Position = matrix * vec4(position, 0.0, 1.0f);
}
