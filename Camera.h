#pragma once
#include <SDL2\SDL_events.h>
#include <glm\ext\vector_float3.hpp>
#include <glm\ext\matrix_float4x4.hpp>
#include <glm\ext\matrix_transform.hpp>
#include <iostream>

class Renderer;

class Camera
{
public:
	Camera(glm::vec3 pos, glm::vec3 angles);

	void update(Renderer* renderer);

	bool checkInputs(SDL_Event _event);

	glm::vec3 getFront();

	glm::mat4 getViewMatrix();

	glm::vec3 getPosition();
	void setPosition(glm::vec3 pos);

	bool moveForward;
	bool moveRight;
	bool moveBackwards;
	bool moveLeft;

	float prevMouseX;
	float prevMouseY;

	float playerHeight = 2.5;
	bool jumped = false;
	const float GRAVITY = 9.81f / 60.0f / 10.0f;
	const float JUMP_POWER = 0.3f;
	const float MAX_SPEED = 0.5f;

private:
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 angles;
};

