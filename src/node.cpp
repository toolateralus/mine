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
#include <climits>
#include <iostream>
#include <memory>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/yaml.h>

vec3 Node::fwd() const { return glm::normalize(vec3(transform[2])); }
vec3 Node::left() const { return glm::normalize(vec3(transform[0])); }
vec3 Node::up() const { return glm::normalize(vec3(transform[1])); }

mat4 Node::get_transform() const {
  glm::mat4 parentTransform = glm::mat4(1.0f);
  if (auto parentNode = parent.lock()) {
    parentTransform = parentNode->get_transform();
  }
  return parentTransform * transform;
}
vec3 Node::get_position() const {
  vec3 position = vec3(transform[3][0], transform[3][1], transform[3][2]);
  if (auto parentNode = parent.lock()) {
    glm::vec4 parentPosition =
        parentNode->get_transform() * glm::vec4(position, 1.0f);
    position = vec3(parentPosition.x, parentPosition.y, parentPosition.z);
  }
  return position;
}
quat Node::get_rotation() const {
  glm::quat rotation = glm::quat_cast(glm::mat3(transform));
  if (auto parentNode = parent.lock()) {
    glm::quat parentRotation = parentNode->get_rotation();
    rotation = parentRotation * rotation;
  }
  return rotation;
}
vec3 Node::get_scale() const {
  vec3 scale = vec3(transform[0][0], transform[1][1], transform[2][2]);
  if (auto parentNode = parent.lock()) {
    vec3 parentScale = parentNode->get_scale();
    scale *= parentScale;
  }
  return scale;
}

void Node::set_position(const vec3 &position) {
  if (auto parentNode = parent.lock()) {
    glm::vec4 parentPosition =
        glm::inverse(parentNode->get_transform()) * glm::vec4(position, 1.0f);
    transform[3][0] = parentPosition.x;
    transform[3][1] = parentPosition.y;
    transform[3][2] = parentPosition.z;
  } else {
    transform[3][0] = position.x;
    transform[3][1] = position.y;
    transform[3][2] = position.z;
  }
}
void Node::scale(const vec3 &scale) { set_scale(get_scale() * scale); }
void Node::translate(const vec3 &translation) {
  set_position(get_position() + translation);
}
void Node::rotate(const glm::quat &rotation) {
  set_rotation(rotation * get_rotation());
}
void Node::set_rotation(const glm::quat &rotation) {
  if (auto parentNode = parent.lock()) {
    glm::quat parentRotation = glm::inverse(parentNode->get_rotation());
    glm::quat localRotation = parentRotation * rotation;
    glm::mat3 rotationMatrix = glm::mat3_cast(localRotation);
    transform[0] = glm::vec4(rotationMatrix[0], transform[0].w);
    transform[1] = glm::vec4(rotationMatrix[1], transform[1].w);
    transform[2] = glm::vec4(rotationMatrix[2], transform[2].w);
  } else {
    glm::mat3 rotationMatrix = glm::mat3_cast(rotation);
    transform[0] = glm::vec4(rotationMatrix[0], transform[0].w);
    transform[1] = glm::vec4(rotationMatrix[1], transform[1].w);
    transform[2] = glm::vec4(rotationMatrix[2], transform[2].w);
  }
}
void Node::set_scale(const vec3 &scale) {
  if (auto parentNode = parent.lock()) {
    vec3 parentScale = parentNode->get_scale();
    vec3 localScale = scale / parentScale;
    transform[0][0] = localScale.x;
    transform[1][1] = localScale.y;
    transform[2][2] = localScale.z;
  } else {
    transform[0][0] = scale.x;
    transform[1][1] = scale.y;
    transform[2][2] = scale.z;
  }
}
void Node::awake() {
  for (auto component : components) {
    if (!component->is_awake) {
      component->is_awake = true;
      component->awake();
    }
  }
}
void Node::update(float dt) {
  for (auto component : components) {
    // skip uninitialized components? why do we need this.
    if (component) {
      awake();
      component->update(dt);
    } else {
      std::cout << "Component is null" << std::endl;
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
    auto scene = engine.m_scene;
    this->name = in["name"].as<std::string>();
    auto transform = in["transform"];
    this->transform = string_to_mat4(transform.as<std::string>());
    auto components = in["components"];
    for (auto component : components) {
      auto type = component["type"].as<std::string>();
        
      if (type == "Light") {
        auto light = this->add_component<Light>();
        light->deserialize(component);
        
        if (engine.m_scene->light == nullptr) {
            engine.m_scene->light = shared_from_this();
        }
      }
      if (type == "Camera") {
        auto camera = this->add_component<Camera>();
        camera->deserialize(component);
        
        if (engine.m_scene->camera == nullptr) {
            engine.m_scene->camera = shared_from_this();
        }
      }
      if (type == "MeshRenderer") {
        auto renderer = this->add_component<MeshRenderer>();
        renderer->deserialize(component);
      }
      if (type == "Rigidbody") {
        auto rigidbody = this->add_component<physics::Rigidbody>();
        engine.m_physics->rigidbodies.push_back(rigidbody);
        rigidbody->deserialize(component);
      }
      if (type == "Collider") {
        auto collider = this->add_component<physics::Collider>();
        engine.m_physics->colliders.push_back(collider);
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
    out << YAML::Key << "transform" << YAML::Value << mat4_to_string(transform);
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
    scene->new_node_queue.push_back(node);
    return node;
}
void Node::add_child(shared_ptr<Node> child) {
  auto &engine = Engine::current();
  shared_ptr<Scene> &scene = engine.m_scene;
  scene->remove_node(child);
  if (has_cyclic_inclusion(child)) {
    std::cout << "Cyclic inclusion detected, not adding child" << std::endl;
    return;
  }
  children.push_back(child);
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
