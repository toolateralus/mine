#pragma once
#include "usings.hpp"
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <yaml-cpp/emitter.h>

struct Node;
namespace physics {
  struct Collision;
}
class Component {
public:
  Component() {};
  // TODO: implement a smarter way of registering components for deserialization : 
  // right now it has to be hand implemented.
  // this 'factories' setup is probably a decent choice.
  
  // static std::unordered_map<std::type_index, std::function<shared_ptr<Component>()>> factories;
  // static void register_component(const std::type_index &type, const std::function<shared_ptr<Component>()> &factory) {
  //   factories[type] = factory;
  // }
  // static bool is_registered(const std::type_index &type) {
  //   return factories.find(type) != factories.end();
  // }
  bool is_awake = false;
  weak_ptr<Node> node;
  virtual ~Component() {}
  virtual void awake() = 0;
  virtual void update(const float &dt) = 0;
  virtual void serialize(YAML::Emitter &out) = 0;
  virtual void deserialize(const YAML::Node &in) = 0;
  
  virtual void on_gui() {}
  virtual void on_collision(const physics::Collision &collision) {}
};


