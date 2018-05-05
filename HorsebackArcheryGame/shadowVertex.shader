#version 330 core

//Basic shader solely used to get depth values for shadow map.
layout (location = 0) in vec3 position;

uniform mat4 shadow_model_matrix;
uniform mat4 shadow_view_matrix;
uniform mat4 shadow_projection_matrix;

void main() {
	gl_Position = shadow_projection_matrix * shadow_view_matrix * shadow_model_matrix * vec4(position.x, position.y, position.z, 1.0);
}