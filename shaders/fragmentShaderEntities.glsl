#version 330 core

in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 texCoords;
in vec3 toCameraVector;
in float visibility;

layout(location = 0) out vec4 outColor;

uniform sampler2D diffuseTex;

float ambientLighting = 0.2f;
float specularStrength = 0.5f;
vec3 skyColor = vec3(0, 0.4f, 0.7f); //should be uniform
vec3 lightColor = vec3(1f, 0.9f, 0.9f); //should be uniform

void main() {
	//lighting calc
	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitLightVector = normalize(toLightVector);

	float nDotl = dot(unitNormal, unitLightVector);
	float brightness = max(nDotl, 0.0);

	//Textures
	vec4 textureColor = texture2D(diffuseTex, texCoords);
	if(textureColor.a < 0.5){
		discard; //fake transparency for grass
	}

	//Phong lighting
	vec3 ambient = ambientLighting * lightColor;
	vec3 diffuse = brightness * lightColor;
	//Specular lighting (done in view space)
	vec3 unitVectorToCamera = normalize(toCameraVector);
	vec3 lightDirection = -unitLightVector;// -unitVectorToCamera; //which one right??
	vec3 reflectedLightDirection = reflect(lightDirection, unitNormal);
	float spec = pow(max(dot(unitVectorToCamera, reflectedLightDirection), 0.0), 32); //32 is shineness
	vec3 specular = specularStrength * spec * lightColor; 

	outColor = vec4(ambient + diffuse + specular, 1.0) * textureColor;

	//Distance fog
	outColor = mix(vec4(skyColor, 1.0), outColor, visibility);
}; 