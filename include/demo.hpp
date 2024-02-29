#pragma once

#include "camera.hpp"
#include "component.hpp"
#include "mesh.hpp"
#include "node.hpp"
#include "engine.hpp"
#include "tostring.hpp"

#include "input.hpp"

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/fwd.hpp>
#include <iostream>
#include <ostream>


struct BlockPlacer : public Component {
  BlockPlacer() {}
  ~BlockPlacer() override {}
  bool placed;
  vector<shared_ptr<Node>> placed_blocks = {};
  shared_ptr<Material> textured_material;
  void awake() override;
  void update(const float &dt) override;
};

// this file is used for testing components, making temporary classes etc.
class Player : public Camera {
public:
  Player() {
    
  }
  ~Player() override {}
  void awake() override {}
  void update(const float &dt) override;
};

static void setup_cube_scene() {
  // this is the area of the cube of cubes that will spawn in a grid.
  const int spawn_area = 4;
  
  const vec3 spawn_offset = vec3(10, 5, 10);
  
  auto &m_engine = Engine::current();
  
  auto m_scene = m_engine.m_scene;
  auto m_physics = m_engine.m_physics;
  auto m_renderer = m_engine.m_renderer;
  auto m_material = m_engine.m_material;
  
  auto WORLD_ORIGIN = m_scene->add_node();
  
  m_renderer->add_mesh(WORLD_ORIGIN, m_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/cube.obj");
  
  for (int x = 0; x < spawn_area; x++) {
    for (int y = 0; y < spawn_area; y++) {
      for (int z = 0; z < spawn_area; z++) {
        auto position = vec3(x, y, z) + spawn_offset;
        
        auto node = m_scene->add_node(position);
        
        auto rigidbody = m_physics->add_rigidbody(node);
        
        auto collider = m_physics->add_collider(node);
            
        auto mesh = m_renderer->add_mesh(
            node, m_material,
            Engine::RESOURCE_DIR_PATH + "/prim_mesh/cube.obj");
      }
    }
  }
}
static void setup_floor() {
  auto &m_engine = Engine::current();
  auto m_scene = m_engine.m_scene; 
  auto m_physics = m_engine.m_physics;
  auto m_renderer = m_engine.m_renderer;
  auto m_material = m_engine.m_material;
  
  auto floor =
      m_scene->add_node(vec3(0, -1, 0), quat(1, 0, 0, 0), vec3(100, 1, 100));
  m_physics->add_collider(floor);
  auto floor_mesh = m_renderer->add_mesh(
      floor, m_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/cube.obj");
  floor_mesh->color = vec4(0.5, 0.5, 0.5, 1.0f);
}
static void setup_player() {
  auto m_scene = Engine::current().m_scene; 
  auto player_node = m_scene->add_node(vec3(0, 3.5, 15));
  player_node->add_component<BlockPlacer>();
  m_scene->camera = player_node;
  player_node->add_component<Player>();
  player_node->add_component<Camera>();
  player_node->rotate(vec3(0, -45, 0));
}
