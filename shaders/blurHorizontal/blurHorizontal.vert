#version 150

uniform mat4 world_matrix;
uniform mat4 view_projection_matrix;

in vec3 location;
in vec2 texcoord;

out vec2 out_texcoord;

void main(void) {
    out_texcoord = texcoord;

    gl_Position = (view_projection_matrix) * (world_matrix * vec4(location, 1));
}