#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VertexShaderOut
{
    vec3 fragment_position;
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
} vs_out[3];

out GeometryShaderOut
{
    vec3 fragment_position;
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
} gs_out;

void main()
{
//    vec3 p0 = vs_out[0].fragment_position_in_world_coordinates;
//    vec3 p1 = vs_out[1].fragment_position_in_world_coordinates;
//    vec3 p2 = vs_out[2].fragment_position_in_world_coordinates;
//    vec3 normal = normalize(cross(p1 - p0, p2 - p1));
    vec3 normal = (vs_out[0].normal + vs_out[1].normal + vs_out[2].normal) / 3.0f;

    gs_out.fragment_position = vs_out[0].fragment_position;
    gs_out.fragment_position_in_world_coordinates = vs_out[0].fragment_position_in_world_coordinates;
//    gs_out.normal = vs_out[0].normal;
    gs_out.normal = normal;
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    gs_out.fragment_position = vs_out[1].fragment_position;
    gs_out.fragment_position_in_world_coordinates = vs_out[1].fragment_position_in_world_coordinates;
//    gs_out.normal = vs_out[1].normal;
    gs_out.normal = normal;
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    gs_out.fragment_position = vs_out[2].fragment_position;
    gs_out.fragment_position_in_world_coordinates = vs_out[2].fragment_position_in_world_coordinates;
//    gs_out.normal = vs_out[2].normal;
    gs_out.normal = normal;
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();

    EndPrimitive();
}
