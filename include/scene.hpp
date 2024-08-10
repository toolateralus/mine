#pragma once
#include "usings.hpp"

#include <yaml-cpp/yaml.h>

struct Node;
class Scene {
  
public:
  Scene(const Scene &) = delete;
  Scene(Scene &&) = delete;
  Scene &operator=(const Scene &) = delete;
  Scene &operator=(Scene &&) = delete;
  Scene() {}
  
  vector<shared_ptr<Node>> nodes;
  vector<shared_ptr<Node>> new_node_queue;
  shared_ptr<Node> camera;
  shared_ptr<Node> light;
  
  
  void remove_node(shared_ptr<Node> &node);
  void update(const float &dt);
  void on_gui();
  void serialize(YAML::Emitter &out);
  void deserialize(const YAML::Node &in);
};