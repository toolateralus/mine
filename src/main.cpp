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
#include "../include/fileio.hpp"

#include <iostream>
#include <memory>
#include <system_error>
#include <yaml-cpp/emitter.h>

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
        Node::instantiate(vec3(0, -10, 0),vec3(100, 10, 100));
    m_physics->add_collider(floor);
    auto floor_mesh = floor->add_component<MeshRenderer>(m_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/cube.obj");
    floor_mesh->color = vec4(0.5, 0.5, 0.5, 1.0f);
  }
  // SETUP CUBE SCENE
  if (false)
  {
    // this is the area of the cube of cubes that will spawn in a grid.
    const int spawn_area = 4;
    const vec3 spawn_offset = vec3(10, 5, 10);
    
    for (int x = 0; x < spawn_area; x++) {
      for (int y = 0; y < spawn_area; y++) {
        for (int z = 0; z < spawn_area; z++) {
          auto position = vec3(x, y, z) + spawn_offset;
          auto node = Node::instantiate(position);
          auto rigidbody = m_physics->add_rigidbody(node);
          auto collider = m_physics->add_collider(node);
          node->add_component<MeshRenderer>(m_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/cube.obj");
        }
      }
    }
  }
  // SETUP PLAYER
  {
    const auto color = vec3(1, 1, 1);
    const auto intensity = 10.0f;
    const auto range = 1.0;
    const auto cast_shadows = false;
    auto player_node = Node::instantiate(vec3(0, 3.5, 15));
    player_node->set_position(vec3(0,15,0));
    auto light_component = player_node->add_component<Light>(color, intensity, range, cast_shadows);
    m_scene->light = player_node;
    player_node->add_component<BlockPlacer>();
    m_scene->camera = player_node;
    player_node->add_component<Player>();
    player_node->add_component<Camera>();
    player_node->rotate(vec3(0, -45, 0));
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