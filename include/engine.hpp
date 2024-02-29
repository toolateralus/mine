#pragma once

#include "renderer.hpp"
#include "usings.hpp"
#include "input.hpp"
#include "physics.hpp"
#include "scene.hpp"
#include <glm/gtx/quaternion.hpp>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
USING_GLM_TYPES
USING_SMART_POINTERS

struct Engine {
  shared_ptr<Renderer> m_renderer;
  shared_ptr<Scene> m_scene;
  shared_ptr<Input> m_input;
  shared_ptr<physics::Physics> m_physics;
  // these are just quick and easy defaults / fallbacks
  shared_ptr<Shader> m_shader;
  std::optional<shared_ptr<Texture>> m_texture;
  shared_ptr<Material> m_material;
  bool running = true;
  static std::string RESOURCE_DIR_PATH;
  static Engine &current();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
private:
  static void update_loop(const float &dt);
  Engine();
};