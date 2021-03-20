#version 330 core

in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 texCoords;
in vec3 toCameraVector;
in float visibility;

layout(location = 0) out vec4 outColor;

uniform sampler2D texture_diffuse1;

float ambientLighting = 0.2f;
float specularStrength = 0.5f;    //Should read from specular map
float shininess = 32.0f;			
vec3 skyColor = vec3(0.5f, 0.4f, 0.3f); //should be uniform
vec3 lightColor = vec3(1f, 0.9f, 0.9f); //should be uniform

void main() {
	//normalize them
	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitLightVector = normalize(toLightVector);
	vec3 unitVectorToCamera = normalize(toCameraVector);

	//Textures
	vec4 textureColor = texture2D(texture_diffuse1, texCoords);
	//if(textureColor.a < 0.1){
		//discard; //fake transparency for grass
	//}

	/* Phong lighting (done in WORLD space) */
	// Ambient
	vec3 ambient = ambientLighting * lightColor;
	// Diffuse
	float brightness = max(0.0, dot(unitNormal, unitLightVector));
	vec3 diffuse = brightness * lightColor;
	// Specular lighting 
	vec3 lightDirection = -unitLightVector; 
	vec3 reflectedLightDirection = reflect(lightDirection, unitNormal);
	float spec = pow(max(dot(unitVectorToCamera, reflectedLightDirection), 0.0), shininess);
	vec3 specular = specularStrength * spec * lightColor; 

	outColor = vec4(ambient + diffuse + specular, 1.0) * textureColor;

	//Distance fog
	outColor = mix(vec4(skyColor, 1.0), outColor, visibility);
}; 