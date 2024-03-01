#include "../include/collider.hpp"
#include "../include/renderer.hpp"
#include "../include/engine.hpp"
#include "../include/tostring.hpp"
#include <cmath>
#include <glm/matrix.hpp>
#include <iostream>

using namespace physics;

vector<vec3> Collider::get_points() {
  auto node = this->node.lock();
  if (!node) {
    return vector<vec3>();
  }
  
  points.clear();
  points.resize(8);
  
  // why in the world does dividing by 2 cause collision to totally break and 
  // produce nans???
  float halfX = size.x / 2.0f;
  float halfY = size.y / 2.0f;
  float halfZ = size.z / 2.0f;
  
  points[0] = vec3(halfX, halfY, halfZ);
  points[1] = vec3(-halfX, halfY, halfZ);
  points[2] = vec3(-halfX, -halfY, halfZ);
  points[3] = vec3(halfX, -halfY, halfZ);
  points[4] = vec3(halfX, halfY, -halfZ);
  points[5] = vec3(-halfX, halfY, -halfZ);
  points[6] = vec3(-halfX, -halfY, -halfZ);
  points[7] = vec3(halfX, -halfY, -halfZ);
  
  for (auto &point : points) {
    point = node->get_transform() * vec4(point, 1.0f);
    if (point.x == NAN || point.y == NAN || point.z == NAN) {
      std::cout << "Error: point is NAN" << std::endl;
    }
  }
  
  return points;
}
vector<vec3> Collider::get_axes() {
  axes.clear();
  for (size_t i = 0; i < points.size(); i++) {
    for (size_t j = i + 1; j < points.size(); j++) {
      vec3 axis = glm::normalize(points[j] - points[i]);
      axes.push_back(axis);
    }
  }
  return axes;
}
BoundingBox Collider::get_bounds() {
  if (points.size() != 8) {
    this->transform_collider();
  }
  
  bounds = BoundingBox(points[0], points[0]);
  for (const auto &point : points) {
    bounds.expand(point);
  }
  return bounds;
}
void Collider::project(const vec3 &axis, float &min, float &max) {
  if (points.size() != 8) {
    this->transform_collider();
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
  points = get_points();
  axes = get_axes();
  bounds = get_bounds();
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
vec3 physics::Collider::get_closest_point_to(const vec3 &point) {
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
    std::cout << "Error: No closest point found" << std::endl;
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
