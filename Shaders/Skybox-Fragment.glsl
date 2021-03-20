#version 400 core

in vec3 textureCoords;
out vec4 out_Color;

uniform samplerCube cubeMap;
uniform bool fadeHorizon;

const float lowerLimit = 0.0f;
const float upperLimit = 30.0f;

vec3 skyColor = vec3(0.5f, 0.4f, 0.3f); // SHOULD BE UNIFORM

void main(void){
	
	out_Color = texture(cubeMap, textureCoords);

	if (fadeHorizon) {
		float factor = (textureCoords.y - lowerLimit) / (upperLimit - lowerLimit);
		factor = clamp(factor, 0.0, 1.0);
		out_Color = mix(vec4(skyColor, 1.0f), out_Color, factor);
	}
}



