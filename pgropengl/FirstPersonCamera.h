#pragma once
#include "Camera.h"
#include <glm/glm.hpp>

class FirstPersonCamera : public Camera {
public:
    FirstPersonCamera(glm::vec3 initPosition, float movementSpeed, float cameraSensitivity);

    virtual glm::mat4 getViewMatrix() const override;
    virtual glm::vec3 getCameraPosition() const override;

    void MoveForward();
    void MoveBackward();
    void MoveRight();
    void MoveLeft();
    void mouseLook(glm::vec2 mouseDelta);

private:
    glm::vec3 eye;
    glm::vec3 viewDirection;
    glm::vec3 up;
    float cameraSensitivity;
    float movementSpeed;
};