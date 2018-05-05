#version 330 core
 
//Where [a, b, c, d, e, f, g, h]
//a, b and c are the position coordinates
//d and e are the texture coordinates
//f, g and h are the normal coordinates (for lighting).
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture;
layout (location = 2) in vec3 normal;

//Matrices used to influence camera.
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

//Matrices used to influence shadows.
uniform mat4 shadow_view_matrix;
uniform mat4 shadow_projection_matrix;

//Sent to fragment shader.
out vec4 colorPosition;
out vec2 textureCoordinate;
out vec3 normalCoordinate;
out vec4 colorPositionInLight;

void main()
{
	colorPosition = model_matrix * vec4(position.x, position.y, position.z, 1.0); //Basis for when the color of an object is changed (solely used for calculation of normals).
    textureCoordinate = texture;
	normalCoordinate = mat3(transpose(inverse(model_matrix))) * normal; //Allows lighting to be changed when objects change position.
	colorPositionInLight = shadow_projection_matrix * shadow_view_matrix * colorPosition;
	gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position.x, position.y, position.z, 1.0); //Camera position.
}