#include "../include/demo.hpp"
#include "../thirdparty/imgui/imgui.h"

void BlockPlacer::update(const float &dt) {
    auto node = this->node.lock();
    auto &input = Input::current();
    auto &engine = Engine::current();
    if (placed && input.mouse_button_up(MouseButton::Left)) {
      placed = false;
    }
    
    if (!placed && input.mouse_button_down(MouseButton::Left)) {
      placed = true;
      
      if (placed_blocks.size() < 25) {
        auto position = node->get_position() + (node->fwd() * -15.0f);
        auto new_node = engine.m_scene->add_node(position);
        
        if (input.key_down(Key::LeftShift)) {
          engine.m_renderer->add_mesh(new_node, textured_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/cube.obj");
        }
        else {
          engine.m_renderer->add_mesh(new_node, textured_material, Engine::RESOURCE_DIR_PATH + "/prim_mesh/car.obj");
        }
        engine.m_physics->add_rigidbody(new_node);
        engine.m_physics->add_collider(new_node)->draw_collider = true;
        placed_blocks.push_back(new_node);
      } else {
        auto block = placed_blocks.front();
        block->set_position(node->get_position() + (node->fwd() * -15.0f));
        placed_blocks.erase(placed_blocks.begin());
        placed_blocks.push_back(block);
      }
    }
}
void BlockPlacer::awake() {
    auto &engine = Engine::current();
    
    textured_material =
        make_shared<Material>(engine.m_shader, engine.m_texture);
}
void Player::on_gui() {
  ImGui::Begin("Player");
  auto fps = Engine::current().m_renderer->framerate;  
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
      const glm::vec2 delta = input.mouse_delta();
      
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

