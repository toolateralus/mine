#pragma once
#include "component.hpp"
#include <glm/ext/matrix_transform.hpp>

namespace physics {
  struct Collision;
};

class Node : public std::enable_shared_from_this<Node> {
private:
  mat4 transform;

public:
  std::string name;
  vector<shared_ptr<Component>> components;
  
  weak_ptr<Node> parent;
  vector<shared_ptr<Node>> children;
  
  Node() {
    transform = glm::identity<mat4>();
    components = vector<shared_ptr<Component>>();
  }
  ~Node() { components.clear(); }
  void awake();
  void update(float dt);
  void on_collision(const physics::Collision &collision);
  
  // for some reason these seem to be backwards.
  vec3 fwd() const;
  vec3 left() const;
  vec3 up() const;
  
  mat4 get_transform() const;
  vec3 get_position() const;
  quat get_rotation() const;
  vec3 get_scale() const;
  
  void set_position(const vec3 &position);
  void set_rotation(const quat &rotation);
  void set_scale(const vec3 &scale);
  
  void translate(const vec3 &translation);
  void rotate(const quat &rotation);
  void scale(const vec3 &scale);
  
  template <typename T, typename... Args>
  shared_ptr<T> add_component(Args &&...args) {
    auto component = make_shared<T>(args...);
    components.push_back(component);
    component->node = shared_from_this();
    return component;
  }
  template <typename T> void remove_component() {
    for (int i = 0; i < components.size(); i++) {
      if (std::dynamic_pointer_cast<T>(components[i])) {
        components.erase(components.begin() + i);
        return;
      }
    }
  }
  template <typename T> shared_ptr<T> get_component() {
    for (int i = 0; i < components.size(); i++) {
      if (std::dynamic_pointer_cast<T>(components[i])) {
        return std::dynamic_pointer_cast<T>(components[i]);
      }
    }
    return nullptr;
  }
};
