#pragma once
#include <SDL2\SDL_events.h>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "terrain.h"

class Camera
{
public:
	Camera(glm::vec3 pos, glm::vec3 angles) {
		this->position = glm::vec3(pos);
		this->velocity = glm::vec3(0);
		this->acceleration = glm::vec3(0);
		this->angles = glm::vec3(angles);
		//should use array
		moveForward = false;
		moveRight = false;
		moveBackwards = false;
		moveLeft = false;

		this->prevMouseX = -1;
		this->prevMouseY = -1;
	}

	void update(Terrain* terrain) {
		glm::vec3 front = getFront();
		glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));

		glm::vec3 total(0, 0, 0);
		if (moveForward)
			total += front;
		if (moveBackwards)
			total -= front;
		if (moveRight)
			total += right;
		if (moveLeft)
			total -= right;
		this->acceleration.x = total.x;
		this->acceleration.z = total.z;

		// Clamp velocity
		if (abs(this->velocity.x) + abs(this->velocity.z) > this->MAX_SPEED) {
			this->position += this->velocity;
			this->velocity.y += this->acceleration.y;
			this->acceleration.y -= this->GRAVITY;
		}
		else {
			this->position += this->velocity;
			this->velocity += this->acceleration;
			this->acceleration.y -= this->GRAVITY;
		}

		this->velocity *= 0.85;					// Friction

		if (terrain == NULL) return;
		float groundPos = this->playerHeight + terrain->getHeightAt((int)this->position[2], (int)this->position[0]);
		//collide
		if (this->position[1] <= groundPos) {
			this->position[1] = groundPos;
			this->velocity[1] = 0;
			this->acceleration[1] = 0;
			this->jumped = false;
		}
		else {
			this->jumped = true;
		}
	}

	bool checkInputs(SDL_Event _event) {
		//quit game
		if (_event.type == SDL_QUIT)
			return true;

		//keyboard movement
		if (_event.type == SDL_KEYDOWN || _event.type == SDL_KEYUP) {
			bool activate = false;
			if (_event.type == SDL_KEYDOWN)
				activate = true;

			switch (_event.key.keysym.sym) {
			case SDLK_w:
				moveForward = activate;
				break;
			case SDLK_a:
				moveLeft = activate;
				break;
			case SDLK_s:
				moveBackwards = activate;
				break;
			case SDLK_d:
				moveRight = activate;
				break;
			case SDLK_SPACE:
				if (!this->jumped) {
					this->jumped = true;
					this->velocity[1] += this->JUMP_POWER;
				}
				break;
			case SDLK_ESCAPE:
				return true;
				break;
			}
		}
		//mouse movement
		if (_event.type == SDL_MOUSEMOTION) {
			if (prevMouseX == -1) { //if mouse just moved into window
				prevMouseX = (float)_event.motion.x;
				prevMouseY = (float)_event.motion.y;
				return false;
			}
			float xoffset = _event.motion.x - prevMouseX;
			float yoffset = prevMouseY - _event.motion.y;
			prevMouseX = (float)_event.motion.x;
			prevMouseY = (float)_event.motion.y;

			float sensitivity = 0.25;

			angles[0] += yoffset * sensitivity;
			angles[1] += xoffset * sensitivity;
		}
		return false;
	}

	glm::vec3 getFront() {
		//angles to vector
		float pitch = angles[0];
		float yaw = angles[1];

		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		return glm::normalize(front);
	}

	glm::mat4 getViewMatrix() {
		glm::vec3 front = getFront();
		return glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
	}

	glm::vec3 getPosition() {
		return this->position;
	}

	void setPosition(glm::vec3 pos) {
		this->position = pos;
	}

	glm::vec3 getAngles() {
		return this->angles;
	}


	// world
	const float GRAVITY = 9.81f / 60.0f / 12.0f;

	// player
	const float JUMP_POWER = 4.0f;
	const float MAX_SPEED = 2.0f;
	const float playerHeight = 9.0f;

	// Camera
	const float FOV = 75.0f;

	bool moveForward;
	bool moveRight;
	bool moveBackwards;
	bool moveLeft;

	float prevMouseX;
	float prevMouseY;

	bool jumped = false;

private:
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 angles;
};

















