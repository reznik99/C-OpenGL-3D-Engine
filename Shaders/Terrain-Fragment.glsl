#version 330 core

in vec3 surfaceNormal;
in vec3 toLightVector;
in vec2 texCoords;
in vec3 toCameraVector;
in float visibility;

layout(location = 0) out vec4 outColor;

uniform sampler2D blendMapTex;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;

float ambientLighting = 0.2f;
float specularStrength = 0.5f;    //Should read from specular map
float shininess = 16.0f;		
float tiling = 120.0f;
vec3 skyColor = vec3(0.5f, 0.4f, 0.3f); //should be uniform
vec3 lightColor = vec3(1f, 0.9f, 0.9f); //should be uniform

void main() {
	//normalize them
	vec3 unitNormal = normalize(surfaceNormal);
	vec3 unitLightVector = normalize(toLightVector);
	vec3 unitVectorToCamera = normalize(toCameraVector);

	//Textures / Blendmap
	vec4 blendMapColour = texture(blendMapTex, texCoords);
 	float backTextureAmount = 1 - (blendMapColour.r + blendMapColour.g + blendMapColour.b);
 	vec2 tiledCoords = texCoords * tiling;

	vec4 tex1Col = texture2D(tex1, tiledCoords) * backTextureAmount;
	vec4 tex2Col = texture2D(tex2, tiledCoords) * blendMapColour.r;
	vec4 tex3Col = texture2D(tex3, tiledCoords) * blendMapColour.g;
	vec4 tex4Col = texture2D(tex4, tiledCoords) * blendMapColour.b;

	vec4 totalColour = tex1Col + tex2Col + tex3Col + tex4Col;

	//Make tiles shinier than other terrains (This could be loaded from a separate specularMap
	//if(blendMapColour.g > blendMapColour.r && blendMapColour.g > blendMapColour.b){
		//specularStrength = 1.0f;
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

	outColor = vec4(ambient + diffuse + specular, 1.0) * totalColour;

	//Distance fog
	outColor = mix(vec4(skyColor, 1.0), outColor, visibility);
}; 