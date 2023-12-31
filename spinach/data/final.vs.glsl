#version 330

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texcoord;

out vec2 v_texcoord;

void main() {
	gl_Position = vec4(a_position, 0, 1);
	v_texcoord  = a_texcoord;
}