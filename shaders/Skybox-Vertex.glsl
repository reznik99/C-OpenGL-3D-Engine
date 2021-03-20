#version 400 core

in vec3 position;
out vec3 textureCoords;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

void main(void){

	gl_Position = projMatrix * viewMatrix * vec4(position, 1.0);
	textureCoords = position;
}