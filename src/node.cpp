#include "../include/component.hpp"

#include "../include/light.hpp"
#include "../include/camera.hpp"
#include "../include/renderer.hpp"
#include "../include/physics.hpp"
#include "../include/collider.hpp"
#include "../include/engine.hpp"
#include "../include/demo.hpp"

#include "../include/node.hpp"
#include "../include/physics.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>
#include <glm/gtx/quaternion.hpp>

namespace glm {
  mat4 compose(const vec3 &translation, const quat &rotation, const vec3 &scale) {
    mat4 out = glm::translate(glm::identity<mat4>(), translation);
    out *= glm::toMat4(glm::normalize(rotation));
    return glm::scale(out, scale);
  }
}

vec3 Node::fwd() const { return glm::normalize(vec3(local_transform[2])); }
vec3 Node::left() const { return glm::normalize(vec3(local_transform[0])); }
vec3 Node::up() const { return glm::normalize(vec3(local_transform[1])); }

mat4 Node::get_transform() {
  glm::mat4 parentTransform = glm::mat4(1.0f);
  if (auto parentNode = parent.lock()) {
    parentTransform = parentNode->get_transform();
  }
  return parentTransform * get_local_transform();
}
vec3 Node::get_position() {
  return vec3(get_transform()[3]);
}
quat Node::get_rotation() {
  vec3 scale, translation, skew;
  quat rotation;
  vec4 perspective;
  glm::decompose(get_transform(), scale, rotation, translation, skew, perspective);
  return rotation;
}
vec3 Node::get_scale() {
  vec3 scale, translation, skew;
  quat rotation;
  vec4 perspective;
  glm::decompose(get_transform(), scale, rotation, translation, skew, perspective);
  return scale;
}

void Node::set_transform(const mat4 &transform) {
  if (auto parentNode = parent.lock()) {
    set_local_transform(transform * glm::inverse(parentNode->get_transform()));
  } else {
    set_local_transform(transform);
  }
}
void Node::set_position(const vec3 &position) {
  if (auto parentNode = parent.lock()) {
    set_local_position(vec4(position, 0) * glm::inverse(parentNode->get_transform()));
  } else {
    set_local_position(position);
  }
}
void Node::set_rotation(const glm::quat &rotation) {
  if (auto parentNode = parent.lock()) {
    set_local_rotation(glm::toMat4(rotation) * glm::inverse(parentNode->get_transform()));
  } else {
    set_local_rotation(rotation);
  }
}
void Node::set_scale(const vec3 &scale) {
  if (auto parentNode = parent.lock()) {
    auto scale_mat4 = glm::scale(glm::identity<mat4>(), scale) * glm::inverse(parentNode->get_transform());
    vec3 new_scale, translation, skew;
    quat rotation;
    vec4 perspective;
    glm::decompose(scale_mat4, new_scale, rotation, translation, skew, perspective);
    set_local_scale(new_scale);
  } else {
    set_local_scale(scale);
  }
}
void Node::scale(const vec3 &scale) { set_scale(get_scale() * scale); }
void Node::translate(const vec3 &translation) {
  set_position(get_position() + translation);
}
void Node::rotate(const glm::quat &rotation) {
  set_rotation(rotation * get_rotation());
}
void Node::awake() {
  for (auto &component : components) {
    if (component)
      if (!component->is_awake) {
        component->is_awake = true;
        component->awake();
      }
  }
}
void Node::update(float dt) {
  if (new_child_queue.size() != 0) {
    for (auto &new_child : new_child_queue) {
      children.push_back(new_child);
    }
    new_child_queue.clear();
  }
  if (new_component_queue.size() != 0) {
    for (auto &new_component : new_component_queue) {
      components.push_back(new_component);
    }
    new_component_queue.clear();
  }
  for (auto &component : components) {
    if (component) {
      awake();
      component->update(dt);
    } else {
      cout << "Component is null" << std::endl;
    }
  }
  for (auto &child : children) {
    child->update(dt);
  }
}
void Node::on_collision(const physics::Collision &collision) {
  for (auto &component : components) {
    component->on_collision(collision);
  }
}
void Node::on_gui() {
    for (auto &component : components) {
      component->on_gui();
    }
}
void Node::deserialize(const YAML::Node &in) {
    auto &engine = Engine::current();
    auto &scene = engine.m_scene;
    this->name = in["name"].as<std::string>();
    auto transform = in["transform"];
    this->local_transform = string_to_mat4(transform.as<std::string>());
    auto components = in["components"];
    for (auto component : components) {
      auto type = component["type"].as<std::string>();
        
      if (type == "Light") {
        auto light = this->add_component<Light>();
        light->deserialize(component);
        
        if (engine.m_scene.light == nullptr) {
            engine.m_scene.light = shared_from_this();
        }
      }
      if (type == "Camera") {
        auto camera = this->add_component<Camera>();
        camera->deserialize(component);
        
        if (engine.m_scene.camera == nullptr) {
            engine.m_scene.camera = shared_from_this();
        }
      }
      if (type == "MeshRenderer") {
        auto renderer = this->add_component<MeshRenderer>();
        renderer->deserialize(component);
      }
      if (type == "Rigidbody") {
        auto rigidbody = this->add_component<physics::Rigidbody>();
        engine.m_physics.rigidbodies.push_back(rigidbody);
        rigidbody->deserialize(component);
      }
      if (type == "BoxCollider") {
        auto collider = this->add_component<physics::BoxCollider>();
        engine.m_physics.colliders.push_back(collider);
        collider->deserialize(component);
      }
      if (type == "BlockPlacer") {
        auto block_placer = this->add_component<BlockPlacer>();
        block_placer->deserialize(component);
      }
      if (type == "Player") {
        auto player = this->add_component<Player>();
        player->deserialize(component);
      }
    }
    auto children = in["children"];
    for (auto child : children) {
      auto node = Node::instantiate();
      node->parent = shared_from_this();
      node->deserialize(child);
    }
}
void Node::serialize(YAML::Emitter &out) {
    out << YAML::BeginMap;
    out << YAML::Key << "name" << YAML::Value << name;
    out << YAML::Key << "transform" << YAML::Value << mat4_to_string(local_transform);
    out << YAML::Key << "children" << YAML::Value << YAML::BeginSeq;
    for (auto &child : children) {
      child->serialize(out);
    }
    out << YAML::EndSeq;
    out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;
    for (auto &component : components) {
      component->serialize(out);
    }
    out << YAML::EndSeq;
    out << YAML::EndMap;
}
shared_ptr<Node> Node::instantiate(const vec3 &pos, const vec3 &scale,
                                   const quat &rot) {
    auto &engine = Engine::current();
    auto &scene = engine.m_scene;
    auto node = make_shared<Node>();
    node->set_position(pos);
    node->set_rotation(rot);
    node->set_scale(scale);
    scene.new_node_queue.push_back(node);
    return node;
}
void Node::add_child(shared_ptr<Node> child) {
  auto &engine = Engine::current();
  Scene &scene = engine.m_scene;
  scene.remove_node(child);
  if (has_cyclic_inclusion(child)) {
    cout << "Cyclic inclusion detected, not adding child" << std::endl;
    return;
  }
  new_child_queue.push_back(child);
  child->parent = shared_from_this();
}
bool Node::has_cyclic_inclusion(const shared_ptr<Node> &new_child) const {
  std::unordered_set<shared_ptr<Node>> visited = {new_child};
  std::unordered_set<shared_ptr<Node>> recursionStack;
  
  for (const auto &child : children) {
     if (has_cyclic_inclusion_helper(child, visited, recursionStack)) {
        return true;
     }
  }

  return false;
}
bool Node::has_cyclic_inclusion_helper(
    const shared_ptr<Node> &node, std::unordered_set<shared_ptr<Node>> &visited,
    std::unordered_set<shared_ptr<Node>> &recursionStack) const {
  if (recursionStack.count(node)) {
     return true; // Found a cycle
  }

  if (visited.count(node)) {
     return false; // Already visited, no cycle
  }

  visited.insert(node);
  recursionStack.insert(node);

  for (const auto &child : node->children) {
     if (has_cyclic_inclusion_helper(child, visited, recursionStack)) {
        return true;
     }
  }

  recursionStack.erase(node);
  return false;
}
void Node::decompose() {
  this->transform_composed = false;
  glm::decompose(local_transform, local_scale, local_rotation, local_translation, local_skew, local_perspective);
}
void Node::compose() {
  local_transform = glm::compose(local_translation, local_rotation, local_scale);
  this->transform_composed = true;
}
mat4 Node::get_local_transform() {
  if (!this->transform_composed) {
    compose();
  }
  return local_transform;
}
vec3 Node::get_local_position() {
  if (this->transform_composed) {
    decompose();
  }
  return local_translation;
}
quat Node::get_local_rotation() {
  if (this->transform_composed) {
    decompose();
  }
  return local_rotation;
}
vec3 Node::get_local_scale() {
  if (this->transform_composed) {
    decompose();
  }
  return local_scale;
}
void Node::set_local_transform(const mat4 &transform) {
  local_transform = transform;
  this->transform_composed = true;
}
void Node::set_local_position(const vec3 &position) {
  if (this->transform_composed) {
    decompose();
  }
  local_translation = position;
}
void Node::set_local_rotation(const quat &rotation) {
  if (this->transform_composed) {
    decompose();
  }
  local_rotation = rotation;
}
void Node::set_local_scale(const vec3 &scale) {
  if (this->transform_composed) {
    decompose();
  }
  local_scale = scale;
}
