#include "FreeLookCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

	FreeLookCamera::FreeLookCamera(glm::vec3 initPosition, float movementSpeed, float cameraSensitivity) {
		this->cameraSensitivity = cameraSensitivity;
		this->movementSpeed = movementSpeed / 10;
		eye = initPosition;
		initEye = initPosition;
		viewDirection = glm::vec3(0.f, 0.f, -1.f);
		up = glm::vec3(0.f, 1.f, 0.f);
	}

	glm::mat4 FreeLookCamera::getViewMatrix() const {
		return glm::lookAt(eye, eye + viewDirection, up);
	}

	glm::vec3 FreeLookCamera::getCameraPosition() const {
		return eye;
	}

	void FreeLookCamera::resetCamera() {
		eye = initEye;
		viewDirection = glm::vec3(0.f, 0.f, -1.f);
		up = glm::vec3(0.f, 1.f, 0.f);
	}

	void FreeLookCamera::MoveForward() {
		eye += movementSpeed * viewDirection;
	}

	void FreeLookCamera::MoveBackward() {
		eye -= movementSpeed * viewDirection;
	}

	void FreeLookCamera::MoveRight() {
		eye += movementSpeed * glm::cross(viewDirection, up);
	}

	void FreeLookCamera::MoveLeft() {
		eye -= movementSpeed * glm::cross(viewDirection, up);
	}

	void FreeLookCamera::mouseLook(glm::vec2 mouseDelta) {
		glm::vec3 right = glm::cross(viewDirection, up);
		viewDirection = glm::rotate(viewDirection, glm::radians((float)mouseDelta.x * cameraSensitivity), up);
		viewDirection = glm::rotate(viewDirection, glm::radians((float)mouseDelta.y * cameraSensitivity), right);
	}

	