#pragma once
#include "tostring.hpp"
#include "usings.hpp"
#include "component.hpp"
#include "yaml-cpp/yaml.h"
class Camera : public Component {
public:
  float fovy = 60.0f;
  float aspect = 800.0f / 600.0f;
  float near = 0.1f;
  float far = 10000.0f;
  vec3 sky_color = {0.1, 0.3, 0.8};
  
  Camera();
  ~Camera() override {
    // virtual destructor necessary but useless in this case.
  }
  
  void awake() override {}
  void update(const float &dt) override {}
  void serialize(YAML::Emitter &out) override {
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << "Camera";
    out << YAML::Key << "fov" << YAML::Value << fovy;
    out << YAML::Key << "aspect" << YAML::Value << aspect;
    out << YAML::Key << "near" << YAML::Value << near;
    out << YAML::Key << "far" << YAML::Value << far;
    out << YAML::Key << "sky_color" << YAML::Value << YAML::Flow << vec3_to_string(sky_color);
    out << YAML::EndMap;
  }
  void deserialize(const YAML::Node &in) override {
    fovy = in["fov"].as<float>();
    aspect = in["aspect"].as<float>();
    near = in["near"].as<float>();
    far = in["far"].as<float>();
    sky_color = string_to_vec3(in["sky_color"].as<std::string>());
  }
  
  mat4 get_projection();
  mat4 get_view();
  mat4 get_view_projection();
};