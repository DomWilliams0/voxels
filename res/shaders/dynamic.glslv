#version 330 core
layout (location = 0) in vec3 vertex_pos;
layout (location = 1) in vec4 vertex_colour;
layout (location = 2) in mat4 transform;
out vec4 rgba;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * transform * vec4(vertex_pos, 1.0);
    rgba = vertex_colour;
}