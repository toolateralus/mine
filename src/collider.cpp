#include "../include/collider.hpp"
#include "../include/renderer.hpp"
#include "../include/engine.hpp"
#include "../include/tostring.hpp"
#include <cmath>
#include <glm/matrix.hpp>
#include <iostream>

using namespace physics;

vector<vec3> BoxCollider::get_world_points() {
  auto node = this->node.lock();
  if (!node) {
    return vector<vec3>();
  }
  
  if (points.size() != local_points.size()) {
    points.resize(local_points.size());
  }
  
  for (size_t i = 0; i < local_points.size(); i++) {
    points[i] = node->get_transform() * vec4(local_points[i], 1.0f);
  }
    
  for (auto &point : points) {
    if (std::isnan(point.x) || std::isnan(point.y) || std::isnan(point.z)){
      std::cerr << "Error: collider point is NAN" << std::endl;
    }
  }
  
  return points;
}
vector<vec3> Collider::get_world_axes() {
  axes.clear();
  for (size_t i = 0; i < points.size(); i++) {
    for (size_t j = i + 1; j < points.size(); j++) {
      vec3 axis = glm::normalize(points[j] - points[i]);
      axes.push_back(axis);
    }
  }
  return axes;
}
BoundingBox Collider::get_world_bounds() {
  if (points.size() == 0) {
    return {};
  }
  
  bounds = BoundingBox(points[0], points[0]);
  for (const auto &point : points) {
    bounds.expand(point);
  }
  return bounds;
}
void Collider::project(const vec3 &axis, float &min, float &max) {
  if (points.size() == 0) {
    return;
  }
  min = max = glm::dot(points[0], axis);
  for (size_t i = 1; i < points.size(); i++) {
    float projection = glm::dot(points[i], axis);
    min = std::min(min, projection);
    max = std::max(max, projection);
  }
}
void Collider::transform_collider() {
  auto node = this->node.lock();
  if (!node) {
    return;
  }
  is_dirty = false;
  points = get_world_points();
  axes = get_world_axes();
  bounds = get_world_bounds();
}
void physics::Collider::update(const float &dt) {
  if (auto node = this->node.lock()) {
    if (last_position != node->get_position()) {
      is_dirty = true;
    }
    last_position = node->get_position();
    
    if (is_dirty) {
      transform_collider();
    }
  }
  
  if (draw_collider) {
    Gizmo gizmo = Gizmo(node);
    vector<float> vertices;
    
    auto node = this->node.lock();
    
    auto i = 0;
    for (const auto &vert : points) {
      ++i;
      
      auto v = glm::inverse(node->get_transform()) * vec4(vert, 1.0f);
      
      vertices.push_back(v.x);
      vertices.push_back(v.y);
      vertices.push_back(v.z);
    }
    
    static const vector<unsigned int> indices = {
      // Front face
      0, 2, 1, 0, 3, 2,
      // Back face
      4, 6, 5, 4, 7, 6,
      // Left face
      1, 6, 5, 1, 2, 6,
      // Right face
      0, 7, 4, 0, 3, 7,
      // Top face
      0, 5, 1, 0, 4, 5,
      // Bottom face
      2, 7, 6, 2, 3, 7
    };
    gizmo.vertices = vertices;
    gizmo.indices = indices;
    gizmo.color = gizmo_color;
    
    auto &renderer = Engine::current().m_renderer;
    renderer->add_gizmo(gizmo);
  }
}
vector<vec3> physics::Collider::get_indices() {
  vector<vec3> indices;
  for (size_t i = 0; i < points.size(); i += 2) {
    indices.push_back(points[i]);
    indices.push_back(points[i + 1]);
  }
  return indices;
}
vec3 Collider::get_closest_point_to(const vec3 &point) {
  float min = FLT_MAX;
  vec3 closest;
  for (const auto &point : points) {
    float distance = glm::distance(point, point);
    if (distance < min) {
      min = distance;
      closest = point;
    }
  }
  if (min == FLT_MAX) {
    cout << "Error: No closest point found" << std::endl;
    return glm::zero<vec3>();
  }
  return closest;
}
void Collider::on_collision(const Collision &collision) {
  
}
void physics::Collider::serialize(YAML::Emitter &out) {
  out << YAML::BeginMap;
  out << YAML::Key << "type" << YAML::Value << "Collider";
  out << YAML::Key << "center" << YAML::Value << YAML::Flow
      << vec3_to_string(center);
  out << YAML::Key << "size" << YAML::Value << YAML::Flow
      << vec3_to_string(size);
  out << YAML::EndMap;
}
void physics::Collider::deserialize(const YAML::Node &in) {
  center = string_to_vec3(in["center"].as<std::string>());
  size = string_to_vec3(in["size"].as<std::string>());
}
void physics::BoxCollider::deserialize(const YAML::Node &in) {
  auto center = in["center"].as<std::string>();
  auto size = in["size"].as<std::string>();
  this->center = string_to_vec3(center);
  this->size = string_to_vec3(size);
};
void physics::BoxCollider::serialize(YAML::Emitter &out) {
  out << YAML::Key << "type" << YAML::Value << "BoxCollider";
  out << YAML::Key << "center" << YAML::Value << vec3_to_string(center);
  out << YAML::Key << "size" << YAML::Value << vec3_to_string(size);
}
