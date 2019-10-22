#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 angles) {
	this->position = glm::vec3(pos);
	this->angles = glm::vec3(angles);
	//should use array
	moveForward = false;
	moveRight = false;
	moveBackwards = false;
	moveLeft = false;

	this->prevMouseX = -1;
	this->prevMouseY = -1;
}

void Camera::update() {
	float speed = 0.1f;
	glm::vec3 front = getFront();
	glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));

	if (moveForward) {
		this->position += front * speed;
	}if (moveRight) {
		this->position += right * speed;
	}if (moveBackwards) {
		this->position -= front * speed;
	}if (moveLeft) {
		this->position -= right * speed;
	}

	//this->position[1] = 1; //keep player level on ground
}

bool Camera::checkInputs(SDL_Event _event) {
	//quit game
	if (_event.type == SDL_QUIT)
		return true;

	//keyboard movement
	if (_event.type == SDL_KEYDOWN || _event.type == SDL_KEYUP) {
		bool activate = false;
		if(_event.type == SDL_KEYDOWN)
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
		case SDLK_ESCAPE:
			return true;
			break;
		}
	}
	//mouse movement
	if (_event.type == SDL_MOUSEMOTION) {
		if (prevMouseX == -1) { //if mouse just moved into window
			prevMouseX = _event.motion.x;
			prevMouseY = _event.motion.y;
			return false;
		}
		float xoffset = _event.motion.x - prevMouseX;
		float yoffset = prevMouseY - _event.motion.y;
		prevMouseX = _event.motion.x;
		prevMouseY = _event.motion.y;

		float sensitivity = 0.25;

		angles[0] += yoffset * sensitivity;
		angles[1] += xoffset * sensitivity;
	}
	return false;
}

glm::vec3 Camera::getFront() {
	//angles to vector
	float pitch = angles[0];
	float yaw = angles[1];

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	return glm::normalize(front);
}

glm::mat4 Camera::getViewMatrix() {
	glm::vec3 front = getFront();
	return glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
}

glm::vec3 Camera::getPosition() {
	return this->position;
}

