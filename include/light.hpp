#pragma once

#include "component.hpp"

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
};