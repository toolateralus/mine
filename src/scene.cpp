#include "../include/scene.hpp"
#include "../include/node.hpp"

void Scene::update(const float &dt) {
  for (auto node : new_node_queue) {
    nodes.push_back(node);
  }
  new_node_queue.clear();
  
  for (auto node : this->nodes) {

    // lazy awaken components after they're already added to scene graph/ node
    node->awake();

    // update all nodes.
    for (auto component : node->components) {
      component->update(dt);
    }
  }
}

shared_ptr<Node> Scene::add_node(const vec3 &pos, const quat &rot,
                                 const vec3 &scale) {
  auto node = std::make_shared<Node>();
  node->set_position(pos);
  node->set_rotation(rot);
  node->set_scale(scale);
  new_node_queue.push_back(node);
  return node;
}
