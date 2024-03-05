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

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>
#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>

using namespace physics;



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
        Node::instantiate(vec3(0, -10, 0),vec3(1, 1, 1));
    m_physics->add_collider<BoxCollider>(floor, glm::zero<vec3>(), vec3(100, 1, 100));
    auto floor_mesh = floor->add_component<MeshRenderer>(m_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/rooms.obj");
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
      cout << "Loading scene from file : " << scene_path << std::endl;
      m_scene->deserialize(YAML::LoadFile(scene_path));
    } else {
      cout << "Scene file not found, creating default scene\n";
      setup_default_scene();
    }
    m_renderer->run();
    YAML::Emitter out;
    m_scene->serialize(out);
    write_file(scene_path, out.c_str());
  } else {
    cout << "No scene file found, creating default scene\n";
    setup_default_scene();
    m_renderer->run();
  }
  
}