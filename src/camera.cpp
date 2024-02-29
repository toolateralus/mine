#include "../include/camera.hpp"
#include "../include/node.hpp"

// Camera
Camera::Camera() {}

mat4 Camera::get_projection() {
  return glm::perspective(glm::radians(fovy), aspect, near, far);
}
mat4 Camera::get_view() {
  auto node = this->node.lock();
  return glm::inverse(node->get_transform());
}
mat4 Camera::get_view_projection() { return get_projection() * get_view(); }
