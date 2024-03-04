#include "../include/usings.hpp"
#include "../include/input.hpp"
#include <stdexcept>

Input::Input() {}  

Input::~Input() {}

vec2 Input::mouse_position() {
  if (window == nullptr) {
    throw std::runtime_error("Input::mouse_position() called before window was set");
  }
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  return vec2(x, y);
}
vec2 Input::mouse_delta() {
  if (window == nullptr) {
    throw std::runtime_error("Input::mouse_delta() called before window was set");
  }
  
  vec2 pos = mouse_position();
  vec2 delta = pos - last_mouse_pos;
  last_mouse_pos = pos;
  return delta;
}

bool Input::mouse_button_down(MouseButton button) {
  if (window == nullptr) {
    throw std::runtime_error("Input::mouse_button_down() called before window was set");
  }
  return glfwGetMouseButton(window, (int)button) == GLFW_PRESS;
}
bool Input::mouse_button_up(MouseButton button) {
  if (window == nullptr) {
    throw std::runtime_error("Input::mouse_button_up() called before window was set");
  }
  return glfwGetMouseButton(window, (int)button) == GLFW_RELEASE;
}
bool Input::key_down(Key key) {
  if (window == nullptr) {
    throw std::runtime_error("Input::key_down() called before window was set");
  }
  return glfwGetKey(window, (int)key) == GLFW_PRESS;
}
bool Input::key_up(Key key) {
  if (window == nullptr) {
    throw std::runtime_error("Input::key_up() called before window was set");
  }
  return glfwGetKey(window, (int)key) == GLFW_RELEASE;
}