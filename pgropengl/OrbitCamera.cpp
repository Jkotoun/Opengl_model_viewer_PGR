#include "OrbitCamera.h"
#include <glm/gtc/matrix_transform.hpp>

	OrbitCamera::OrbitCamera(glm::vec3 target, float distance, float rotationSpeed, float zoomSpeed, float panSpeed)
		: target(target), distance(distance), rotationSpeed(rotationSpeed), zoomSpeed(zoomSpeed), panSpeed(panSpeed) {
		azimuth = 0.0f;
		elevation = 0.0f;
		initDistance = distance;
		initTarget = target;
		updateCameraPosition();
	}

	glm::mat4 OrbitCamera::getViewMatrix() const  {
		return glm::lookAt(eye, target, up);
	}

	glm::vec3 OrbitCamera::getCameraPosition() const  {
		return eye;
	}
	void OrbitCamera::resetCamera() {
		azimuth = 0.0f;
		elevation = 0.0f;
		distance = initDistance;
		target = initTarget;
		updateCameraPosition();
	}

	void OrbitCamera::rotate(float deltaX, float deltaY) {
		azimuth -= deltaX * rotationSpeed;
		elevation -= deltaY * rotationSpeed;

		// Clamp the elevation to avoid flipping the camera
		elevation = glm::clamp(elevation, -89.0f, 89.0f);

		updateCameraPosition();
	}

	void OrbitCamera::zoom(float deltaZoom) {
		distance -= deltaZoom * zoomSpeed;

		// Clamp the distance to avoid going through the target
		distance = glm::clamp(distance, minDistance, maxDistance);

		updateCameraPosition();
	}

	void OrbitCamera::pan(float deltaX, float deltaY) {
		// Calculate the right vector
		glm::vec3 viewDirection = glm::normalize(target - eye);

		glm::vec3 right = glm::normalize(glm::cross(viewDirection, up));

		// Calculate the translation based on the right and up vectors
		glm::vec3 translation = (right * deltaX + up * deltaY) * panSpeed;

		// Update the target and eye positions
		target += translation;
		eye += translation;

		// Recalculate viewDirection after translation
		viewDirection = glm::normalize(target - eye);
	}


	void OrbitCamera::updateCameraPosition() {
		// Convert azimuth and elevation to radians
		float azimuthRad = glm::radians(azimuth);
		float elevationRad = glm::radians(elevation);

		// Calculate the new position of the camera
		eye.x = target.x + distance * glm::cos(elevationRad) * glm::cos(azimuthRad);
		eye.y = target.y + distance * glm::sin(elevationRad);
		eye.z = target.z + distance * glm::cos(elevationRad) * glm::sin(azimuthRad);
	}

	