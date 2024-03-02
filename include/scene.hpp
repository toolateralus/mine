#pragma once
#include "usings.hpp"

#include <yaml-cpp/yaml.h>

struct Node;
class Scene {
public:
  vector<shared_ptr<Node>> nodes;
  vector<shared_ptr<Node>> new_node_queue;
  shared_ptr<Node> camera;
  shared_ptr<Node> light;
  
  Scene() { nodes = vector<shared_ptr<Node>>(); }
  ~Scene() { nodes.clear(); }
  void update(const float &dt);
  void on_gui();
  void serialize(YAML::Emitter &out);
  void deserialize(const YAML::Node &in);
};