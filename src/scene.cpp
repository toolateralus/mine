#include "../include/scene.hpp"
#include "../include/node.hpp"

void Scene::update(const float &dt) {
  for (const auto &node : new_node_queue) {
    nodes.push_back(node);
  }
  new_node_queue.clear();
  for (auto &node : this->nodes) {
    node->update(dt);
  }
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
  for (auto &node : nodes) {
    auto new_node = Node::instantiate();
    new_node->deserialize(node);
  }
}
void Scene::remove_node(shared_ptr<Node> &node) {
  nodes.erase(std::remove(nodes.begin(), nodes.end(), node), nodes.end());
}
