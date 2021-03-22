#version 330

in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 position_opengl = vec4(position.x, position.z, -position.y, 1.0f);
    gl_Position = projection * view * model * position_opengl;
}
