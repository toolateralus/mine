#include "../include/camera.hpp"
#include "../include/tostring.hpp"
#include <yaml-cpp/yaml.h>

// clang says this is unused but it is : don't know why it's throwing that. we access
// node->get_transform() in get_view() and get_view_projection()
#include "../include/node.hpp"

// Camera
Camera::Camera() {}
void Camera::serialize(YAML::Emitter &out)  {
  out << YAML::BeginMap;
  out << YAML::Key << "type" << YAML::Value << "Camera";
  out << YAML::Key << "fov" << YAML::Value << fovy;
  out << YAML::Key << "aspect" << YAML::Value << aspect;
  out << YAML::Key << "near" << YAML::Value << near;
  out << YAML::Key << "far" << YAML::Value << far;
  out << YAML::Key << "sky_color" << YAML::Value << YAML::Flow
      << vec3_to_string(sky_color);
  out << YAML::EndMap;
}
void Camera::deserialize(const YAML::Node &in)  {
  fovy = in["fov"].as<float>();
  aspect = in["aspect"].as<float>();
  near = in["near"].as<float>();
  far = in["far"].as<float>();
  sky_color = string_to_vec3(in["sky_color"].as<std::string>());
}
mat4 Camera::get_projection() {
  return glm::perspective(glm::radians(fovy), aspect, near, far);
}
mat4 Camera::get_view() {
  auto node = this->node.lock();
  return glm::inverse(node->get_transform());
}
mat4 Camera::get_view_projection() { return get_projection() * get_view(); }
