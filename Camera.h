#pragma once
#include <SDL2\SDL_events.h>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "Terrain.h"
class Renderer;

class Camera
{
public:
	Camera(glm::vec3 pos, glm::vec3 angles);

	void update(Terrain* terrain);

	bool checkInputs(SDL_Event _event);

	glm::vec3 getFront();

	glm::mat4 getViewMatrix();

	glm::vec3 getPosition();
	void setPosition(glm::vec3 pos);
	
	glm::vec3 getAngles();

	bool moveForward;
	bool moveRight;
	bool moveBackwards;
	bool moveLeft;

	float prevMouseX;
	float prevMouseY;

	float playerHeight = 8.0f;
	bool jumped = false;
	const float GRAVITY = 9.81f / 60.0f / 5.0f;
	const float JUMP_POWER = 3.0f;
	const float MAX_SPEED = 1.5f;

private:
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 angles;
};

