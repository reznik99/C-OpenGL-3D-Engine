#version 330 core

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 lightPosition;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoords;

out vec2 texCoords;
out vec3 surfaceNormal;
out vec3 toLightVector;
out vec3 toCameraVector;
out float visibility;

const float density = 0.0006;
const float gradient = 5;

void main() {
	//positions
	vec4 worldPosition = modelMatrix * vec4(vertex, 1.0);
	vec4 positionRelativeToCam = viewMatrix * worldPosition;
	gl_Position = projMatrix * positionRelativeToCam;

	//lighting (world space)
	surfaceNormal = (modelMatrix * vec4(normal, 0.0)).xyz;
	toLightVector = lightPosition - worldPosition.xyz;
	toCameraVector = (inverse(viewMatrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz - worldPosition.xyz;

	//distance fade
	float distance = length(positionRelativeToCam.xyz);
	visibility = exp(-pow((distance*density), gradient));

	texCoords = textureCoords; //pass tex
}