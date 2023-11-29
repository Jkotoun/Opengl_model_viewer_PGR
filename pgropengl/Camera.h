#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    virtual ~Camera() = default;

    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::vec3 getCameraPosition() const = 0;
};
