#pragma once
#include "usings.hpp"

struct Node;

namespace physics {
  struct Collision;
}

class Component {
public:
  Component() {}
  bool is_awake = false;
  weak_ptr<Node> node;
  virtual ~Component() {}
  virtual void awake() = 0;
  virtual void on_gui() {}
  virtual void update(const float &dt) = 0;
  virtual void on_collision(const physics::Collision &collision) {}
};
