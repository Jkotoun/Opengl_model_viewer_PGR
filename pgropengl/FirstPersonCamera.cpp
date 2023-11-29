#include "FirstPersonCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

	FirstPersonCamera::FirstPersonCamera(glm::vec3 initPosition, float movementSpeed, float cameraSensitivity) {
		this->cameraSensitivity = cameraSensitivity;
		this->movementSpeed = movementSpeed / 10;
		eye = initPosition;
		viewDirection = glm::vec3(0.f, 0.f, -1.f);
		up = glm::vec3(0.f, 1.f, 0.f);
	}

	glm::mat4 FirstPersonCamera::getViewMatrix() const {
		return glm::lookAt(eye, eye + viewDirection, up);
	}

	glm::vec3 FirstPersonCamera::getCameraPosition() const {
		return eye;
	}

	void FirstPersonCamera::MoveForward() {
		eye += movementSpeed * viewDirection;
	}

	void FirstPersonCamera::MoveBackward() {
		eye -= movementSpeed * viewDirection;
	}

	void FirstPersonCamera::MoveRight() {
		eye += movementSpeed * glm::cross(viewDirection, up);
	}

	void FirstPersonCamera::MoveLeft() {
		eye -= movementSpeed * glm::cross(viewDirection, up);
	}

	void FirstPersonCamera::mouseLook(glm::vec2 mouseDelta) {
		glm::vec3 right = glm::cross(viewDirection, up);
		viewDirection = glm::rotate(viewDirection, glm::radians((float)mouseDelta.x * cameraSensitivity), up);
		viewDirection = glm::rotate(viewDirection, glm::radians((float)mouseDelta.y * cameraSensitivity), right);
	}