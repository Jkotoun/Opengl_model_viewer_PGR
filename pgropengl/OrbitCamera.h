#pragma once

#include "Camera.h"
#include <glm/glm.hpp>

class OrbitCamera : public Camera {
public:
    OrbitCamera(glm::vec3 target, float distance, float rotationSpeed, float zoomSpeed, float panSpeed);

    virtual glm::mat4 getViewMatrix() const override;
    virtual glm::vec3 getCameraPosition() const override;
    void rotate(float deltaX, float deltaY);
    void zoom(float deltaZoom);
    void pan(float deltaX, float deltaY);

private:
    void updateCameraPosition();

    glm::vec3 eye;
    glm::vec3 target;
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 viewDirection;
    float azimuth;
    float elevation;
    float distance;
    float rotationSpeed;
    float zoomSpeed;
    float panSpeed;
    const float minDistance = 1.0f;
    const float maxDistance = 100.0f;
};
