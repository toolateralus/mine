
#include "../include/engine.hpp"
#include <filesystem>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

/*
  IMPORTANT : 
    if you intend to debug with RenderDoc or run the program from anywhere but the project root, you must adjust
    this Engine::RESOURCE_DIR_PATH to the correct path to the resources directory.
      
    I just use VScode to right click the folder, get 'copy path' and paste it here.
*/

std::string Engine::RESOURCE_DIR_PATH = std::filesystem::current_path().string() + "/res";

void Engine::update_loop(const float &dt) {
  auto &current = Engine::current();
  auto &m_input = Input::current();
  
  auto &m_scene = current.m_scene;
  auto &m_renderer = current.m_renderer;
  if (m_input.key_down(Key::Home)) {
      current.running = true;
  }
  if (m_input.key_down(Key::End)) {
      current.running = false;
  }
  if (current.running) {
    m_scene.update(dt);
  }
}
Engine::Engine() : m_renderer("Mine Engine", SCREEN_H, SCREEN_W, update_loop), m_input(Input::current()) {
  m_shader = make_shared<Shader>(RESOURCE_DIR_PATH + "/shaders/vertex.glsl",
                                 RESOURCE_DIR_PATH + "/shaders/fragment.glsl");
  m_texture = optional<shared_ptr<Texture>>(
      make_shared<Texture>(RESOURCE_DIR_PATH + "/textures/conflag.jpg"));
  m_material = make_shared<Material>(m_shader, std::nullopt);
  m_input.window = m_renderer.window;
  
};
Engine &Engine::current() {
  static Engine instance = Engine();
  return instance;
}
