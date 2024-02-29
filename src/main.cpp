#include "../include/renderer.hpp"
#include "../include/usings.hpp"
#include "../include/scene.hpp"
#include "../include/component.hpp"
#include "../include/mesh.hpp"
#include "../include/camera.hpp"
#include "../include/node.hpp"
#include "../include/engine.hpp"
#include "../include/input.hpp"
#include "../include/demo.hpp"
#include "../include/physics.hpp"
#include "../include/tostring.hpp"
#include "../include/light.hpp"

#include <iostream>
#include <memory>

using namespace physics;

/*
 IMPORTANT : 
  if you intend to debug with RenderDoc or run the program from anywhere but the project root, you must adjust
  this RESOURCE_DIR_PATH to the correct path to the resources directory.
  
  I just use VScode to right click the folder, get 'copy path' and paste it here.
*/
static const std::string CUBE_MESH_FILE_PATH = Engine::RESOURCE_DIR_PATH + "/prim_mesh/cube.obj";

// We do this to initialize the singletons and provide easy access here.
static Engine &m_engine = Engine::current();
static Input &m_input = Input::current();

shared_ptr<Scene> m_scene = m_engine.m_scene;
shared_ptr<Physics> m_physics = m_engine.m_physics;
shared_ptr<Material> m_material = m_engine.m_material;
shared_ptr<Renderer> m_renderer =  m_engine.m_renderer;

int main(int argc, char **argv) {
  auto light = m_scene->add_node();
  light->set_position(vec3(0,15,0));
  auto light_component = light->add_component<Light>();
  light_component->intensity = 1;
  light_component->range = 1;
  light_component->color = glm::vec3(1.0f, 1.0f, 1.0f);
  m_scene->light = light;
  
  setup_floor();
  //setup_cube_scene();
  setup_player();
  m_renderer->run();
}