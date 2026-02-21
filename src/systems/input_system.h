#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class InputSystem
{
public:
    explicit InputSystem(GLFWwindow *window);
    ~InputSystem();

    InputSystem(const InputSystem &) = delete;
    InputSystem &operator=(const InputSystem &) = delete;
    InputSystem(InputSystem &&) = delete;
    InputSystem &operator=(InputSystem &&) = delete;

    void update();

    // Action mapping
    void createAction(const std::string &name, std::vector<int> keys);
    bool isActionPressed(const std::string &name) const;
    bool isActionJustPressed(const std::string &name) const;
    bool isActionJustReleased(const std::string &name) const;

    // Raw key queries
    bool isKeyPressed(int key) const;
    bool isKeyJustPressed(int key) const;
    bool isKeyJustReleased(int key) const;

    // Mouse buttons
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonJustPressed(int button) const;
    bool isMouseButtonJustReleased(int button) const;

    // Mouse position / delta
    glm::vec2 getMousePosition() const { return mousePos; }
    glm::vec2 getMouseDelta() const { return mouseDelta; }

    // Scroll
    glm::vec2 getScrollDelta() const { return scrollDelta; }

private:
    static constexpr int KEY_COUNT = GLFW_KEY_LAST + 1;
    static constexpr int MOUSE_BUTTON_COUNT = 8;

    GLFWwindow *window;

    // Key state: callbacks write to live, update() snapshots to current/previous
    std::array<bool, KEY_COUNT> liveKeys{};
    std::array<bool, KEY_COUNT> currentKeys{};
    std::array<bool, KEY_COUNT> previousKeys{};

    // Mouse button state
    std::array<bool, MOUSE_BUTTON_COUNT> liveMouseButtons{};
    std::array<bool, MOUSE_BUTTON_COUNT> currentMouseButtons{};
    std::array<bool, MOUSE_BUTTON_COUNT> previousMouseButtons{};

    // Mouse position
    glm::vec2 mousePos{0.0f};
    glm::vec2 lastMousePos{0.0f};
    glm::vec2 mouseDelta{0.0f};

    // Scroll
    glm::vec2 scrollAccumulator{0.0f};
    glm::vec2 scrollDelta{0.0f};

    // Action map: action name -> list of GLFW key codes
    std::unordered_map<std::string, std::vector<int>> actions;

    // Static instance for GLFW callbacks
    static InputSystem *instance;

    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow *window, double xPos, double yPos);
    static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
};
