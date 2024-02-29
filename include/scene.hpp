#pragma once
#include "usings.hpp"
struct Node;
class Scene {
public:
  vector<shared_ptr<Node>> nodes;
  vector<shared_ptr<Node>> new_node_queue;
  shared_ptr<Node> camera;
  shared_ptr<Node> light;
  
  Scene() { nodes = vector<shared_ptr<Node>>(); }
  ~Scene() { nodes.clear(); }
  shared_ptr<Node> add_node(const vec3 &pos = {0, 0, 0},
                            const quat &rot = {1, 0, 0, 0},
                            const vec3 &scale = {1, 1, 1});
  void update(const float &dt);
  void on_gui();
};