#version 330

uniform sampler2D u_diffuse;

in  vec2 v_texcoord;

out vec4 final_color;

void main() {
	final_color = texture(u_diffuse, v_texcoord);
}