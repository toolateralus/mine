#pragma once
#include <algorithm>
#include "component.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <unordered_set>
#include <yaml-cpp/emitter.h>

namespace physics {
struct Collision;
};

class Node : public std::enable_shared_from_this<Node> {
private:
  mat4 local_transform;
  vec3 local_scale = glm::one<vec3>(), local_translation = glm::zero<vec3>(), local_skew;
  quat local_rotation = glm::identity<quat>();
  vec4 local_perspective;
  bool transform_composed = false;
  void decompose();
  void compose();
  bool has_cyclic_inclusion(const shared_ptr<Node> &node) const;
  bool has_cyclic_inclusion_helper(
      const shared_ptr<Node> &node,
      std::unordered_set<shared_ptr<Node>> &visited,
      std::unordered_set<shared_ptr<Node>> &recursionStack) const;
  vector<shared_ptr<Component>> new_component_queue = {};
  vector<shared_ptr<Component>> components;
  vector<shared_ptr<Node>> new_child_queue;
  vector<shared_ptr<Node>> children;
public:
  vector<shared_ptr<Node>> get_children() { 
    return children;
  }
  
  std::string name;
  weak_ptr<Node> parent;
  Node() : local_transform(glm::identity<mat4>()), name("Node"), components() {}
  ~Node() { components.clear(); }
  void awake();
  void update(float dt);
  void on_collision(const physics::Collision &collision);
  void on_gui();
  void serialize(YAML::Emitter &out);
  void deserialize(const YAML::Node &in);
  void add_child(shared_ptr<Node> child);
  
  static shared_ptr<Node> instantiate(const vec3 &pos = glm::zero<vec3>(),
                                      const vec3 &scale = glm::one<vec3>(),
                                      const quat &rot = glm::identity<quat>());
  // for some reason these seem to be backwards.
  vec3 fwd() const;
  vec3 left() const;
  vec3 up() const;

  mat4 get_local_transform();
  vec3 get_local_position();
  quat get_local_rotation();
  vec3 get_local_scale();

  void set_local_transform(const mat4 &transform);
  void set_local_position(const vec3 &position);
  void set_local_rotation(const quat &rotation);
  void set_local_scale(const vec3 &scale);

  mat4 get_transform();
  vec3 get_position();
  quat get_rotation();
  vec3 get_scale();

  void set_transform(const mat4 &transform);
  void set_position(const vec3 &position);
  void set_rotation(const quat &rotation);
  void set_scale(const vec3 &scale);

  void translate(const vec3 &translation);
  void rotate(const quat &rotation);
  void scale(const vec3 &scale);

  template <typename T, typename... Args>
  shared_ptr<T> add_component(Args &&...args) {
    auto component = make_shared<T>(args...);
    component->node = shared_from_this();
    new_component_queue.push_back(component);
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
  template <typename T> void remove_component(shared_ptr<T> component) {
    auto it = std::ranges::find(components, component);
    if (it != components.end()) {
      components.erase(it);
    }
    
  }
  template <typename T> shared_ptr<T> get_component() {
    for (int i = 0; i < components.size(); i++) {
      if (std::dynamic_pointer_cast<T>(components[i])) {
        return std::dynamic_pointer_cast<T>(components[i]);
      }
    }
    // we need to check the new component queue as well, 
    // components are added to the queue and then added to the components list at the beginning of the next frame,
    // this would be strange if the components didn't exist until the next frame, but it's a possibility someone
    // wants to add and get on the same frame.
    for (int i = 0; i < new_component_queue.size(); i++) {
      if (std::dynamic_pointer_cast<T>(new_component_queue[i])) {
        return std::dynamic_pointer_cast<T>(new_component_queue[i]);
      }
    }
    return nullptr;
  }
};
