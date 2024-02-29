#pragma once
#include "usings.hpp"
#include "component.hpp"

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
  
  mat4 get_projection();
  mat4 get_view();
  mat4 get_view_projection();
};