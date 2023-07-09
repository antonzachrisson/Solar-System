#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_texcoord;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_world;

out vec2 v_texcoord;

void main() {
	gl_Position = u_projection * u_view * u_world * vec4(a_position, 1);
	v_texcoord = a_texcoord;
}