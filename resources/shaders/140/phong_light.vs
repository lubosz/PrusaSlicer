#version 140

uniform mat4 view_model_matrix;
uniform mat4 projection_matrix;

in vec3 v_position;
in vec3 v_normal;

out vec4 position_view;
out vec3 normal_view;

void main()
{
    normal_view = v_normal;

    position_view = view_model_matrix * vec4(v_position, 1.0);

    gl_Position = projection_matrix * position_view;
}
