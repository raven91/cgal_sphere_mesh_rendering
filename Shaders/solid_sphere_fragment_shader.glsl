#version 330

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

in GeometryShaderOut
{
    vec3 fragment_position;
    vec3 fragment_position_in_world_coordinates;
    vec3 normal;
} gs_out;

out vec4 resulting_color;

uniform vec3 view_pos;

uniform mat4 model;

void main()
{
    float ambient_strength = 1.0f;
    vec3 light_color = vec3(1.0f, 1.0f, 1.0f);
    vec3 ambient_color = light.ambient * material.ambient;
    vec3 ambient = ambient_strength * ambient_color;

    float diffuse_strength = 2.0f;
    vec3 diffuse_color = light.diffuse * material.diffuse;
    vec3 normalized_normal = normalize(gs_out.normal);
    vec3 light_direction = normalize(light.position - gs_out.fragment_position_in_world_coordinates);
    float difference = max(dot(light_direction, normalized_normal), 0.0f);
    vec3 diffuse = diffuse_strength * difference * diffuse_color;

    float specular_strength = 0.2f;
    vec3 specular_color = light.specular * material.specular;
    vec3 view_direction = normalize(view_pos - gs_out.fragment_position_in_world_coordinates);
//    vec3 view_direction = normalize(view_pos - vec3(model*vec4(gs_out.fragment_position_in_world_coordinates,1.0f))); // if light is given in the coordinates of the simulation
    vec3 reflect_direction = reflect(-light_direction, normalized_normal);
//    reflect_direction = vec3(model * vec4(reflect_direction, 1.0f)); // if light is given in the coordinates of the simulation
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0f), material.shininess);
    vec3 specular = specular_strength * spec * specular_color;

    resulting_color = vec4(ambient + diffuse + specular, 1.0f);
}
