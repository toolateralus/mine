#include "../include/physics.hpp"
#include <yaml-cpp/yaml.h>
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <unordered_set>

using namespace physics;

int Physics::PHYSICS_OCTREE_MAX_LEVELS = 25;
int Physics::MAX_OBJECTS = 50;

Physics::Physics() {}
Physics::~Physics() {}
void Physics::update(const float &dt) {
  for (auto &rb : rigidbodies) {
    rb->apply_drag(dt);
    rb->integrate(dt);
  }
  // Collision Detection / Resolution
  
  // clear & rebuild the spatial partitioning tree
  root.clear();
  for (auto &collider : colliders) {
    if (auto node = collider->node.lock()) {
      root.insert(node);
    }
  }
  
  
  for (auto &collider : colliders) {
    auto node = collider->node.lock();
    if (!node) {
      continue;
    }

    auto bounds = collider->bounds;
    auto query_result = root.query(bounds);

    // check broad-phase collision : query spatial partitioning tree
    for (auto &other : query_result) {
      if (node == other) {
        continue;
      }
      auto other_collider = other->get_component<Collider>();
      if (other_collider) {
        auto other_bounds = other_collider->bounds;

        // narrow phase : check if the two colliders are intersecting
        auto projection = sat_project(collider, other_collider);
        auto mtv = projection.mtv;

        if (mtv != glm::zero<vec3>()) {
          physics::Collision collision(node, other, projection);
          resolve_collision_discrete(collision, dt, node, other, collider,
                                     other_collider);
          other->on_collision(collision);
          node->on_collision(collision);
        }
      }
    }
  }
}

SATProjection Physics::sat_project(shared_ptr<Collider> &a_collider,
                                   shared_ptr<Collider> &b_collider) {
  vector<vec3> axes = a_collider->axes;
  vector<vec3> axes2 = b_collider->axes;
  axes.insert(axes.end(), axes2.begin(), axes2.end());
  float overlap = FLT_MAX;
  vec3 smallest;
  SATProjection projection;
  
  if (axes.size() == 0) {
    return projection;
  }
  
  for (auto &axis : axes) {
    float min1, max1, min2, max2;
    a_collider->project(axis, min1, max1);
    b_collider->project(axis, min2, max2);

    float current_overlap = std::min(max1, max2) - std::max(min1, min2);

    if (current_overlap < 0) {
      return projection;
    } else {
      if (fabs(current_overlap) < fabs(overlap)) {
        overlap = current_overlap;
        smallest = axis;
      }
    }
  }

  vec3 a_center = a_collider->get_center();
  vec3 b_center = b_collider->get_center();

  vec3 direction = b_center - a_center;
  vec3 mtv = smallest * overlap;

  if (glm::dot(direction, mtv) < 0) {
    mtv = -mtv;
  }

  projection.mtv = mtv;
  projection.normal = glm::normalize(mtv);

  // rough aproximation
  projection.point = a_collider->get_closest_point_to(b_center + mtv);

  return projection;
}

// perform a SAT test to resolve the collision
void Physics::resolve_collision_discrete(Collision &collision, const float &dt,
                                         shared_ptr<Node> &node_a,
                                         shared_ptr<Node> &node_b,
                                         shared_ptr<Collider> &a_collider,
                                         shared_ptr<Collider> &b_collider) {

  auto rb_a = node_a->get_component<Rigidbody>();
  auto rb_b = node_b->get_component<Rigidbody>();

  bool a_is_static = !rb_a || rb_a->mass == 0.0f;
  bool b_is_static = !rb_b || rb_b->mass == 0.0f;

  // don't collide if both are static objects
  if (a_is_static && b_is_static)
    return;

  // if one is static, move the other
  if (a_is_static && !b_is_static) {
    resolve_static_to_dynamic_collision(collision, node_b, rb_b);
    return;
  }

  // same as above
  if (b_is_static && !a_is_static) {
    collision.mtv = -collision.mtv;
    resolve_static_to_dynamic_collision(collision, node_a, rb_a);
    return;
  }

  resolve_dynamic_collision(collision, node_a, node_b, rb_a, rb_b);
}
void Physics::resolve_dynamic_collision(const Collision &collision,
                                        shared_ptr<Node> &node_a,
                                        shared_ptr<Node> &node_b,
                                        shared_ptr<Rigidbody> &rb_a,
                                        shared_ptr<Rigidbody> &rb_b) const {
    const auto mtv = collision.mtv;
    const auto normal = collision.normal;
    const auto correction = mtv / 2.0f;
    
    node_a->translate(-correction);
    node_b->translate(correction);
    
    vec3 rel_vel = rb_b->velocity - rb_a->velocity;
    
    // Linear impulse
    float impulse = glm::dot(rel_vel, normal) / (1 / rb_a->mass + 1 / rb_b->mass);
    
    rb_a->velocity += normal * impulse / rb_a->mass;
    rb_b->velocity -= normal * impulse / rb_b->mass;
    
}

void Physics::resolve_static_to_dynamic_collision(const Collision &collision,
                                                  shared_ptr<Node> &node,
                                                  shared_ptr<Rigidbody> &rb) const {
  const auto mtv = collision.mtv;
  node->translate(mtv / 2.0f);
  vec3 normal = glm::normalize(mtv);
  float impulse = glm::dot(rb->velocity, normal) / (1 / rb->mass);
  rb->velocity -= normal * impulse * 0.8f;
}


// TODO: optimize memory allocations here : 
//    we could avoid recursion and use a more efficient structure for the octree child octree container.

Octree::Octree(const int level, const BoundingBox bounds)
    : level(level), bounds(bounds) {}
Octree::~Octree() { clear(); }
void Octree::clear() {
  nodes.clear();
  children.clear();
}
void Octree::insert(shared_ptr<Node> &node) {
  auto point = node->get_position();
  auto scale = node->get_scale();
  
  auto node_bounds =
      BoundingBox{point - (scale / 2.0f), point + (scale / 2.0f)};
  
  if (!bounds.intersects(node_bounds)) {
    return;
  }
  if (!children.empty()) {
    for (auto &child : children) {
      child->insert(node);
    }
  } else {
    nodes.push_back(node);
    if (nodes.size() > Physics::MAX_OBJECTS && level > 0) {
      subdivide();
    }
  }
}
void Octree::subdivide() {
  children.reserve(8);
  for (int i = 0; i < 8; i++) {
    auto newCenter = bounds.get_center();
    auto halfSize = bounds.get_size() / 2.0f;
    newCenter.x += halfSize.x * (i & 4 ? 0.5f : -0.5f);
    newCenter.y += halfSize.y * (i & 2 ? 0.5f : -0.5f);
    newCenter.z += halfSize.z * (i & 1 ? 0.5f : -0.5f);
    children.push_back(std::make_shared<Octree>(
        level - 1, BoundingBox(newCenter, halfSize * 0.5f)));
  }
  for (auto &node : nodes) {
    for (auto &child : children) {
      child->insert(node);
    }
  }
  nodes.clear();
}
vector<shared_ptr<Node>>
Octree::query(const BoundingBox &query_boundary) const {
  vector<shared_ptr<Node>> out_nodes;
  std::unordered_set<shared_ptr<Node>> unique_nodes;

  if (level == 0 || !bounds.intersects(query_boundary)) {
    return out_nodes;
  }

  if (!children.empty()) {
    for (auto &child : children) {
      auto childColliders = child->query(query_boundary);

      for (auto &collider : childColliders) {
        unique_nodes.insert(collider);
      }
    }
  } else {
    for (auto &node : nodes) {
      auto pos = node->get_position();
      auto scale = node->get_scale();
      auto node_bounds =
          BoundingBox{pos - (scale / 2.0f), pos + (scale / 2.0f)};
      if (query_boundary.intersects(node_bounds)) {
        unique_nodes.insert(node);
      }
    }
  }

  out_nodes.assign(unique_nodes.begin(), unique_nodes.end());
  return out_nodes;
}

void Rigidbody::apply_drag(const float &dt) {
  if (velocity.length() > 0.0001f)
    velocity *= drag;
  else if (velocity != glm::zero<vec3>())
    velocity = glm::zero<vec3>();

  if (angular.length() > 0.0001f)
    angular *= drag;
  else if (angular != glm::zero<vec3>())
    angular = glm::zero<vec3>();
}
void Rigidbody::integrate(const float &dt) {
  if (auto node = this->node.lock()) {
    node->translate(velocity * dt);
    node->rotate(glm::quat(angular * dt));
    if (using_gravity) {
      velocity.y -= 9.81f * dt;
    }
  }
}
shared_ptr<Rigidbody> Physics::add_rigidbody(shared_ptr<Node> &node,
                                                      const float mass,
                                                      const float drag) {
  auto rb = node->add_component<Rigidbody>(mass, drag);
  rigidbodies.push_back(rb);
  return rb;
}

void Rigidbody::deserialize(const YAML::Node &in)  {
  mass = in["mass"].as<float>();
  drag = in["drag"].as<float>();
}
void Rigidbody::serialize(YAML::Emitter &out) {
  out << YAML::BeginMap;
  out << YAML::Key << "type" << YAML::Value << "Rigidbody";
  out << YAML::Key << "mass" << YAML::Value << mass;
  out << YAML::Key << "drag" << YAML::Value << drag;
  out << YAML::EndMap;
}
