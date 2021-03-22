#version 330

in vec3 position;
in vec3 normal;

out VertexShaderOut
{
    vec3 fragment_position;
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 position_opengl = vec4(position.x, position.z, -position.y, 1.0f);
    gl_Position = projection * view * model * position_opengl;

    vs_out.fragment_position = vec3(position_opengl);
    // if light is given in OpenGL model coordinates
    vs_out.fragment_position_in_world_coordinates = vec3(model * vec4(position, 1.0f));
    vs_out.normal = mat3(model) * vec3(normal.x, normal.z, -normal.y);
    // if light is given in the coordinates of the simulation
//    vs_out.fragment_position_in_world_coordinates = position;
//    vs_out.normal = vec3(normal.x, normal.z, -normal.y);
}
