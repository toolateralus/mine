#include "../include/renderer.hpp"
#include "../include/usings.hpp"
#include "../include/scene.hpp"
#include "../include/component.hpp"
#include "../include/mesh.hpp"
#include "../include/node.hpp"
#include "../include/engine.hpp"
#include "../include/input.hpp"
#include "../include/demo.hpp"
#include "../include/physics.hpp"
#include "../include/fileio.hpp"

#include <iostream>
#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>

using namespace physics;

/*
 IMPORTANT : 
  if you intend to debug with RenderDoc or run the program from anywhere but the project root, you must adjust
  this RESOURCE_DIR_PATH to the correct path to the resources directory.
  
  I just use VScode to right click the folder, get 'copy path' and paste it here.
*/

// We do this to initialize the singletons and provide easy access here.
static Engine &m_engine = Engine::current();
static Input &m_input = Input::current();

shared_ptr<Scene> &m_scene = m_engine.m_scene;
shared_ptr<Physics> &m_physics = m_engine.m_physics;
shared_ptr<Material> &m_material = m_engine.m_material;
shared_ptr<Renderer> &m_renderer =  m_engine.m_renderer;

void setup_default_scene() {
 
  
  // SETUP FLOOR
  {
    auto floor =
        Node::instantiate(vec3(0, -10, 0),vec3(1000, 10, 1000));
    m_physics->add_collider<BoxCollider>(floor);
    auto floor_mesh = floor->add_component<MeshRenderer>(m_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/cube.obj");
    floor_mesh->color = vec4(0.5, 0.5, 0.5, 1.0f);
  }
  
  // SETUP PLAYER
  {
    
    auto player_node = Node::instantiate();
    player_node->add_component<Car>();
  }
  
}

int main(int argc, char **argv) {
  
  if (argc == 2) {
    std::string scene_path(argv[1]);
    if (file_exists(scene_path)) {
      std::cout << "Loading scene from file : " << scene_path << std::endl;
      m_scene->deserialize(YAML::LoadFile(scene_path));
    } else {
      std::cout << "Scene file not found, creating default scene\n";
      setup_default_scene();
    }
    m_renderer->run();
    YAML::Emitter out;
    m_scene->serialize(out);
    write_file(scene_path, out.c_str());
  } else {
    std::cout << "No scene file found, creating default scene\n";
    setup_default_scene();
    m_renderer->run();
  }
  
}