#pragma once
#include <SDL2\SDL_events.h>
#include <glm\ext\vector_float3.hpp>
#include <glm\ext\matrix_float4x4.hpp>
#include <glm\ext\matrix_transform.hpp>
#include <iostream>

class Camera
{
public:
	Camera(glm::vec3 pos, glm::vec3 angles);

	void update();

	bool checkInputs(SDL_Event _event);

	glm::vec3 getFront();

	glm::mat4 getViewMatrix();

	glm::vec3 getPosition();

	bool moveForward;
	bool moveRight;
	bool moveBackwards;
	bool moveLeft;

	float prevMouseX;
	float prevMouseY;

private:
	glm::vec3 position;
	glm::vec3 angles;
};

