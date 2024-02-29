#pragma once

#include "bbox.hpp"
#include "component.hpp"
#include "node.hpp"
#include "collider.hpp"
#include "tostring.hpp"
#include <iostream>
#include <set>

namespace physics {
constexpr int PHYSICS_OCTREE_MAX_LEVELS = 250;
constexpr int MAX_OBJECTS = 100;

struct Rigidbody : public Component {
  vec3 velocity;
  vec3 angular;
  
  float mass, drag;
  bool using_gravity = true;
  
  Rigidbody(const float mass = 1.0f, const float drag = 0.98f)
      : mass(mass), drag(drag), velocity(), angular() {}
  ~Rigidbody() override {}
  void awake() override {}
  void apply_drag(const float &dt);
  void integrate(const float &dt);
  void update(const float &dt) override {}
  void on_collision(const physics::Collision &collision) override {
    //std::cout << "collision at : " << vec3_to_string(collision.point) << std::endl;
  }
};

struct Octree {
  int level;
  BoundingBox bounds;
  vector<shared_ptr<Octree>> children;
  vector<shared_ptr<Node>> nodes;
  Octree(const int level, const BoundingBox bounds);
  ~Octree();
  void clear();
  void insert(shared_ptr<Node> &collider);
  void subdivide();
  vector<shared_ptr<Node>> query(const BoundingBox &query_boundary) const;
};



struct Physics {
  Octree root =
      Octree(PHYSICS_OCTREE_MAX_LEVELS, BoundingBox(vec3(-1000), vec3(1000)));
  vector<shared_ptr<Collider>> colliders;
  vector<shared_ptr<Rigidbody>> rigidbodies;
  Physics();
  ~Physics();
  void update(const float &dt);
  shared_ptr<Rigidbody> add_rigidbody(shared_ptr<Node> &node,
                                      const float mass = 1.0f,
                                      const float drag = 0.98f);
  
  shared_ptr<Collider> add_collider(shared_ptr<Node> &node,
                                    const vec3 center = {0, 0, 0},
                                    const vec3 size = {1, 1, 1});
  SATProjection sat_project(shared_ptr<Collider> &collider_a,
                                      shared_ptr<Collider> &collider_b);
  void resolve_collision_discrete(Collision &collison, const float &dt, shared_ptr<Node> &a, shared_ptr<Node> &b,
                         shared_ptr<Collider> &collider_a,
                         shared_ptr<Collider> &collider_b);
  void resolve_static_to_dynamic_collision(const Collision &collison, shared_ptr<Node> &node, shared_ptr<Rigidbody> &rb) const;
  void resolve_dynamic_collision(const Collision &collison, shared_ptr<Node> &node_a,
                                        shared_ptr<Node> &node_b,
                                        shared_ptr<Rigidbody> &rb_a,
                                        shared_ptr<Rigidbody> &rb_b) const;
};


};
