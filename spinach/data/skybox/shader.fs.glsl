#version 330

uniform samplerCube u_cubemap;

in  vec3 v_texcoord;
out vec4 final_color;

void main() {
	final_color = texture(u_cubemap, v_texcoord);
}
