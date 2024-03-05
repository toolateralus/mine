#pragma once
#include "bbox.hpp"
#include "component.hpp"
#include "node.hpp"
#include "usings.hpp"

namespace physics {
struct SATProjection {
  bool did_collide() { return mtv == glm::zero<vec3>(); }
  vec3 mtv = {}, point = {}, normal = {};
  ~SATProjection() = default;
  SATProjection() = default;
  SATProjection(const vec3 &mtv, const vec3 &point, const vec3 &normal)
      : mtv(mtv), point(point), normal(normal) {}
};

struct Collision {
  shared_ptr<Node> a, b;
  vec3 mtv, normal, point;
  Collision(shared_ptr<Node> &a, shared_ptr<Node> &b, const SATProjection &proj)
      : a(a), b(b), mtv(proj.mtv), normal(proj.normal), point(proj.point) {}
};



struct Collider : public Component {
  vec3 size;
  vec3 center;
  vector<vec3> local_points, local_axes;
  vector<vec3> points, axes;
  BoundingBox bounds;
  bool is_dirty = true;
  
  bool draw_collider = false;
  vec4 gizmo_color = {1.0f, 0.0f, 0.0f, 1.0f};
  
  vec3 last_position;
  Collider() = default;
  Collider(vec3 center, vec3 size) : size(size), center(center) {}
  ~Collider() override {}
  
  void awake() override {}
  void update(const float &dt) override;
  void on_collision(const physics::Collision &collision) override;
  
  void serialize(YAML::Emitter &out) override;
  void deserialize(const YAML::Node &in) override;
  void transform_collider();
  void project(const vec3 &axis, float &min, float &max);
  vec3 get_closest_point_to(const vec3 &point);
  vec3 get_center() const { return node.lock()->get_position() + center; }
  virtual vector<vec3> get_indices();
  virtual vector<vec3> get_world_axes();
  virtual vector<vec3> get_world_points() = 0;
  BoundingBox get_world_bounds();
};
struct BoxCollider : public Collider {
  BoxCollider() : BoxCollider(zero<vec3>(), one<vec3>()) {}
  BoxCollider(vec3 center, vec3 size) : Collider(center, size) {
    local_points.clear();
    local_points.resize(8);
    
    float halfX = size.x / 2.0f;
    float halfY = size.y / 2.0f;
    float halfZ = size.z / 2.0f;
    
    local_points[0] = vec3(halfX, halfY, halfZ);
    local_points[1] = vec3(-halfX, halfY, halfZ);
    local_points[2] = vec3(-halfX, -halfY, halfZ);
    local_points[3] = vec3(halfX, -halfY, halfZ);
    local_points[4] = vec3(halfX, halfY, -halfZ);
    local_points[5] = vec3(-halfX, halfY, -halfZ);
    local_points[6] = vec3(-halfX, -halfY, -halfZ);
    local_points[7] = vec3(halfX, -halfY, -halfZ);
  }
  ~BoxCollider() override = default;
  vector<vec3> get_world_points() override;
  void serialize(YAML::Emitter &out) override;
  void deserialize(const YAML::Node &in) override;
};
} // namespace physics