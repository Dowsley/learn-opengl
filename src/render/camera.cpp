#include "camera.h"

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(pos, pos + front, up);
}

void Camera::processKeyboard(CameraDirection direction, float deltaTime)
{
    float velocity = speed * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(front, up));

    switch (direction)
    {
        case FORWARD:  pos += front * velocity; break;
        case BACKWARD: pos -= front * velocity; break;
        case LEFT:     pos -= right * velocity; break;
        case RIGHT:    pos += right * velocity; break;
        case UP:       pos.y += velocity; break;
        case DOWN:     pos.y -= velocity; break;
    }
}

void Camera::processMouseMovement(float xOffset, float yOffset)
{
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch = glm::clamp(pitch + yOffset, -89.0f, 89.0f);

    updateFront();
}

void Camera::processScroll(float yOffset)
{
    fov = glm::clamp(fov - yOffset, 1.0f, 45.0f);
}

void Camera::updateFront()
{
    front = glm::normalize(glm::vec3(
        glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
        glm::sin(glm::radians(pitch)),
        glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
    ));
}
