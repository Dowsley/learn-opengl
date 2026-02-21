#include "input_system.h"

InputSystem *InputSystem::instance = nullptr;

InputSystem::InputSystem(GLFWwindow *window) : window(window)
{
    instance = this;

    // Initialize mouse position from current cursor to avoid first-frame jump
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    mousePos = {static_cast<float>(xPos), static_cast<float>(yPos)};
    lastMousePos = mousePos;

    // Register GLFW callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
}

InputSystem::~InputSystem()
{
    if (instance == this)
        instance = nullptr;
}

void InputSystem::update()
{
    // Snapshot key state
    previousKeys = currentKeys;
    currentKeys = liveKeys;

    // Snapshot mouse button state
    previousMouseButtons = currentMouseButtons;
    currentMouseButtons = liveMouseButtons;

    // Compute mouse delta
    mouseDelta = mousePos - lastMousePos;
    lastMousePos = mousePos;

    // Snapshot scroll and reset accumulator
    scrollDelta = scrollAccumulator;
    scrollAccumulator = {0.0f, 0.0f};
}

// --- Action mapping ---

void InputSystem::createAction(const std::string &name, std::vector<int> keys)
{
    actions[name] = std::move(keys);
}

bool InputSystem::isActionPressed(const std::string &name) const
{
    auto it = actions.find(name);
    if (it == actions.end()) return false;
    for (int key : it->second)
        if (isKeyPressed(key)) return true;
    return false;
}

bool InputSystem::isActionJustPressed(const std::string &name) const
{
    auto it = actions.find(name);
    if (it == actions.end()) return false;
    for (int key : it->second)
        if (isKeyJustPressed(key)) return true;
    return false;
}

bool InputSystem::isActionJustReleased(const std::string &name) const
{
    auto it = actions.find(name);
    if (it == actions.end()) return false;
    for (int key : it->second)
        if (isKeyJustReleased(key)) return true;
    return false;
}

// --- Raw key queries ---

bool InputSystem::isKeyPressed(int key) const
{
    if (key < 0 || key >= KEY_COUNT) return false;
    return currentKeys[key];
}

bool InputSystem::isKeyJustPressed(int key) const
{
    if (key < 0 || key >= KEY_COUNT) return false;
    return currentKeys[key] && !previousKeys[key];
}

bool InputSystem::isKeyJustReleased(int key) const
{
    if (key < 0 || key >= KEY_COUNT) return false;
    return !currentKeys[key] && previousKeys[key];
}

// --- Mouse button queries ---

bool InputSystem::isMouseButtonPressed(int button) const
{
    if (button < 0 || button >= MOUSE_BUTTON_COUNT) return false;
    return currentMouseButtons[button];
}

bool InputSystem::isMouseButtonJustPressed(int button) const
{
    if (button < 0 || button >= MOUSE_BUTTON_COUNT) return false;
    return currentMouseButtons[button] && !previousMouseButtons[button];
}

bool InputSystem::isMouseButtonJustReleased(int button) const
{
    if (button < 0 || button >= MOUSE_BUTTON_COUNT) return false;
    return !currentMouseButtons[button] && previousMouseButtons[button];
}

// --- GLFW callbacks (static) ---

void InputSystem::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (!instance || key < 0 || key >= KEY_COUNT) return;
    if (action == GLFW_PRESS)
        instance->liveKeys[key] = true;
    else if (action == GLFW_RELEASE)
        instance->liveKeys[key] = false;
}

void InputSystem::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (!instance || button < 0 || button >= MOUSE_BUTTON_COUNT) return;
    if (action == GLFW_PRESS)
        instance->liveMouseButtons[button] = true;
    else if (action == GLFW_RELEASE)
        instance->liveMouseButtons[button] = false;
}

void InputSystem::cursorPosCallback(GLFWwindow *window, double xPos, double yPos)
{
    if (!instance) return;
    instance->mousePos = {static_cast<float>(xPos), static_cast<float>(yPos)};
}

void InputSystem::scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    if (!instance) return;
    instance->scrollAccumulator += glm::vec2{static_cast<float>(xOffset), static_cast<float>(yOffset)};
}
