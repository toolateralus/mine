#pragma once

#include "component.hpp"
#include "tostring.hpp"
#include <yaml-cpp/yaml.h>

struct Light : public Component {
  vec3 color;
  float intensity;
  float range;
  bool cast_shadows;
  Light(const vec3 &color = {1, 1, 1}, const float &intensity = 1.0f,
        const float &range = 100.0f, const bool &cast_shadows = true)
      : color(color), intensity(intensity), range(range),
        cast_shadows(cast_shadows) {}
  ~Light() override {}
  void awake() override {}
  void update(const float &dt) override {}
  void on_collision(const physics::Collision &collision) override {}
  void serialize(YAML::Emitter &out) override {
    out << YAML::BeginMap;
    out << YAML::Key << "type" << YAML::Value << "Light";
    out << YAML::Key << "color" << YAML::Value << YAML::Flow << vec3_to_string(color);
    out << YAML::Key << "intensity" << YAML::Value << intensity;
    out << YAML::Key << "range" << YAML::Value << range;
    out << YAML::Key << "cast_shadows" << YAML::Value << cast_shadows;
    out << YAML::EndMap;
  }
  void deserialize(const YAML::Node &in) override {
    color = string_to_vec3(in["color"].as<std::string>());
    intensity = in["intensity"].as<float>();
    range = in["range"].as<float>();
    cast_shadows = in["cast_shadows"].as<bool>();
  }
};