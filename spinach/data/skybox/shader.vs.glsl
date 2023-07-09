#version 330

layout(location = 0) in vec3 a_position;

uniform mat4 u_projection;
uniform mat4 u_view;

out vec3 v_texcoord;

void main()
{
	gl_Position = (u_projection * u_view * vec4(a_position, 1));
	v_texcoord = a_position;
}
