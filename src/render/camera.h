#pragma once

#include <glm/glm.hpp>

enum CameraDirection
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera
{
public:
    glm::vec3 pos{0.0f, 0.0f, 3.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};

    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 45.0f;
    float speed = 6.5f;
    float sensitivity = 0.1f;

    glm::mat4 getViewMatrix() const;
    void processKeyboard(CameraDirection direction, float deltaTime);
    void processMouseMovement(float xOffset, float yOffset);
    void processScroll(float yOffset);

private:
    void updateFront();
};
