#include "../include/scene.hpp"
#include "../include/node.hpp"

void Scene::update(const float &dt) {
  for (auto node : new_node_queue) {
    nodes.push_back(node);
  }
  new_node_queue.clear();
  
  for (auto node : this->nodes) {
    node->update(dt);
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

void Scene::on_gui() {
  for (auto &node : nodes) {
    node->on_gui();
  }
}

void Scene::serialize(YAML::Emitter &out) {
  out << YAML::BeginMap;
  out << YAML::Key << "nodes";
  out << YAML::Value << YAML::BeginSeq;
  for (auto &node : nodes) {
    node->serialize(out);
  }
  out << YAML::EndSeq;
  out << YAML::EndMap;
}

void Scene::deserialize(const YAML::Node &in) {
  auto nodes = in["nodes"].as<std::vector<YAML::Node>>();
  for (auto node : nodes) {
    auto new_node = add_node();
    new_node->deserialize(node);
  }
}
