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
  BlockPlacer() {
  }
  ~BlockPlacer() override {}
  bool placed;
  vector<shared_ptr<Node>> placed_blocks = {};
  shared_ptr<Material> textured_material;
  void awake() override;
  void update(const float &dt) override;
  void serialize(YAML::Emitter &out) override;
  void deserialize(const YAML::Node &in) override;
};

// this file is used for testing components, making temporary classes etc.
class Player : public Camera {
public:
  Player() {}
  ~Player() override {}
  void on_gui() override;
  void awake() override {}
  void update(const float &dt) override;
};

static void setup_cube_scene();
static void setup_floor();
static void setup_player();
