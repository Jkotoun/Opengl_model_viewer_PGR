#pragma once
#include "Camera.h"
#include <glm/glm.hpp>

class FreeLookCamera : public Camera {
public:
    FreeLookCamera(glm::vec3 initPosition, float movementSpeed, float cameraSensitivity);

    virtual glm::mat4 getViewMatrix() const override;
    virtual glm::vec3 getCameraPosition() const override;
    virtual void resetCamera() override;

    void MoveForward();
    void MoveBackward();
    void MoveRight();
    void MoveLeft();
    void mouseLook(glm::vec2 mouseDelta);

private:
    glm::vec3 eye;
    glm::vec3 viewDirection;
    glm::vec3 up;
    glm::vec3 initEye;
    float cameraSensitivity;
    float movementSpeed;
};