
#include "../include/engine.hpp"
#include "../include/camera.hpp"
#include "../include/node.hpp"
#include "../include/demo.hpp"
#include "../include/light.hpp"
#include "../thirdparty/imgui/imgui.h"
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>

void BlockPlacer::update(const float &dt) {
  auto node = this->node.lock();
  auto &input = Input::current();
  auto &engine = Engine::current();
  if (placed && input.mouse_button_up(MouseButton::Left)) {
    placed = false;
  }
  if (!placed && input.mouse_button_down(MouseButton::Left)) {
    placed = false;

    if (placed_blocks.size() < 250) {
      auto position = node->get_position() + (node->fwd() * -15.0f);
      auto new_node = Node::instantiate(position);
      
      if (input.key_down(Key::LeftShift)) {
        new_node->add_component<MeshRenderer>(textured_material,
                                              Engine::RESOURCE_DIR_PATH +
                                                  "/prim_mesh/cube.obj");
      } else {
        new_node->add_component<MeshRenderer>(textured_material,
                                              Engine::RESOURCE_DIR_PATH +
                                                  "/prim_mesh/car.obj");
      }
      placed_blocks.push_back(new_node);
    } else {
      auto block = placed_blocks.front();
      block->set_position(node->get_position() + (node->fwd() * -15.0f));
      placed_blocks.erase(placed_blocks.begin());
      placed_blocks.push_back(block);
    }
  } else {
    if (input.mouse_button_down(MouseButton::Right) && placed_blocks.size() != 0) {
      auto block = placed_blocks.back();
      vec2 delta;
      auto mouse_pos = input.mouse_position();
      if (last_pos.has_value()) {
        delta = mouse_pos - last_pos.value();
      } else {
        delta = vec2(0);
      }
	  if (input.key_down(Key::LeftShift)) {
		auto curr_pos = block->get_position();
		block->set_position(curr_pos + (node->left() * delta.x + node->up() * -delta.y) * 0.1f);
	  } else if (input.key_down(Key::LeftControl)) {
		auto curr_scale = block->get_scale();
		block->set_scale(curr_scale + curr_scale * delta.y / 10.0f);
	  } else {
		block->rotate(glm::radians(node->up() * delta.x + node->left() * delta.y));
	  }
      last_pos = mouse_pos;
    } else {
		last_pos.reset();
	}
  }
}
void BlockPlacer::awake() {
  auto &engine = Engine::current();

  textured_material = make_shared<Material>(engine.m_shader, engine.m_texture);
}
void Player::on_gui() {
  ImGui::Begin("Player");
  auto fps = Engine::current().m_renderer.framerate;
  ImGui::Text("FPS: %f", fps);
  ImGui::End();
}
void Player::update(const float &dt) {
  shared_ptr<Node> node = this->node.lock();
  vec3 move_vec = vec3(0);
  Input &input = Input::current();
  // up/down
  {
    if (input.key_down(Key::C)) {
      move_vec += vec3(0, -1.0, 0);
    }
    if (input.key_down(Key::V)) {
      move_vec += vec3(0, 1.0, 0);
    }
  }
  /*  mouse controller
      left click + move : pan / translate
      right click + move : look / rotate
  */
  {
    const float rotationSens = 0.01f;
    const float panSens = 0.1f;
    const vec2 delta = input.mouse_delta();

    if (input.mouse_button_down(MouseButton::Right)) {
      float pitch = -delta.y * rotationSens;
      float yaw = -delta.x * rotationSens;
      auto rotation = glm::angleAxis(yaw, vec3(0, 1, 0)) *
                      glm::angleAxis(pitch, node->left()) *
                      node->get_rotation();
      node->set_rotation(glm::normalize(rotation));
    }

    if (input.mouse_button_down(MouseButton::Left)) {
      vec3 panDirection =
          (-node->left() * delta.x + node->up() * delta.y) * panSens;
      move_vec += panDirection;
    }
  }
  // forward/backward/left/right
  {
    if (input.key_down(Key::W)) {
      move_vec += -node->fwd();
    }
    if (input.key_down(Key::A)) {
      move_vec += -node->left();
    }
    if (input.key_down(Key::S)) {
      move_vec += node->fwd();
    }
    if (input.key_down(Key::D)) {
      move_vec += node->left();
    }
  }
  move_vec *= input.key_down(Key::LeftShift) ? 10.0f : 5.0f;
  node->translate(move_vec * dt);
}

void BlockPlacer::serialize(YAML::Emitter &out) {
  out << YAML::BeginMap;
  out << YAML::Key << "type" << YAML::Value << "BlockPlacer";
  out << YAML::Key << "textured_material";
  out << YAML::Value << textured_material->serialize();
  out << YAML::EndMap;
}
void BlockPlacer::deserialize(const YAML::Node &in) {
  textured_material = make_shared<Material>();
  textured_material->deserialize(in["textured_material"]);
}

void Car::awake() {
  const auto color = vec3(1, 1, 1);
  const auto intensity = 1.0f;
  const auto range = 1.0;
  const auto cast_shadows = false;
  auto self = node.lock();
  
  auto &engine = Engine::current();
  auto &m_scene = engine.m_scene;
  
  // setup light
  auto light = Node::instantiate();
  auto light_component =
      light->add_component<Light>(color, intensity, range, cast_shadows);
  m_scene.light = light;
  light->set_position(vec3(0, 5, 0));
    
  self->add_component<Player>();
  self->add_component<BlockPlacer>();
  
  auto mesh = self->add_component<MeshRenderer>(
      engine.m_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/car.obj");
  
  self->remove_component(mesh);   
  
  auto camera = Node::instantiate();
  m_scene.camera = camera;
  camera->add_component<Camera>();
  camera->set_position(vec3(0.0, 1, 5));

  self->add_child(camera);
  self->add_child(light);
}
void Car::update(const float &dt) {
  auto node = this->node.lock();
  auto &input = Input::current();
  vec3 move_vec = vec3(0);
}
void Car::serialize(YAML::Emitter &out) {
  out << YAML::BeginMap;
  out << YAML::Key << "type" << YAML::Value << "Car";
  out << YAML::EndMap;
}
void Car::deserialize(const YAML::Node &in) {}
