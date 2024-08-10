#pragma once

#include "renderer.hpp"
#include "usings.hpp"
#include "input.hpp"
#include "physics.hpp"
#include "scene.hpp"
#include <glm/gtx/quaternion.hpp>

struct Engine {
  Renderer m_renderer;
  Scene m_scene = Scene();
  Input &m_input;
  physics::Physics m_physics;
  
  // these are just quick and easy defaults / fallbacks
  shared_ptr<Shader> m_shader;
  optional<shared_ptr<Texture>> m_texture;
  shared_ptr<Material> m_material;
  
  bool running = true;
  static std::string RESOURCE_DIR_PATH;
  static Engine &current();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  void on_gui() {
    m_scene.on_gui();
  }
private:
  static void update_loop(const float &dt);
  Engine();
};