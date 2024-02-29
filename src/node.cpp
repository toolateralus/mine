#include "../include/node.hpp"
#include "../include/physics.hpp"
#include <climits>


vec3 Node::fwd() const { return glm::normalize(vec3(transform[2])); }
vec3 Node::left() const { return glm::normalize(vec3(transform[0])); }
vec3 Node::up() const { return glm::normalize(vec3(transform[1])); }

mat4 Node::get_transform() const {
  glm::mat4 parentTransform = glm::mat4(1.0f);
  if (auto parentNode = parent.lock()) {
    parentTransform = parentNode->get_transform();
  }
  return parentTransform * transform;
}
vec3 Node::get_position() const {
  vec3 position = vec3(transform[3][0], transform[3][1], transform[3][2]);
  if (auto parentNode = parent.lock()) {
    glm::vec4 parentPosition =
        parentNode->get_transform() * glm::vec4(position, 1.0f);
    position = vec3(parentPosition.x, parentPosition.y, parentPosition.z);
  }
  return position;
}
quat Node::get_rotation() const {
  glm::quat rotation = glm::quat_cast(glm::mat3(transform));
  if (auto parentNode = parent.lock()) {
    glm::quat parentRotation = parentNode->get_rotation();
    rotation = parentRotation * rotation;
  }
  return rotation;
}
vec3 Node::get_scale() const {
  vec3 scale = vec3(transform[0][0], transform[1][1], transform[2][2]);
  if (auto parentNode = parent.lock()) {
    vec3 parentScale = parentNode->get_scale();
    scale *= parentScale;
  }
  return scale;
}

void Node::set_position(const vec3 &position) {
  if (auto parentNode = parent.lock()) {
    glm::vec4 parentPosition =
        glm::inverse(parentNode->get_transform()) * glm::vec4(position, 1.0f);
    transform[3][0] = parentPosition.x;
    transform[3][1] = parentPosition.y;
    transform[3][2] = parentPosition.z;
  } else {
    transform[3][0] = position.x;
    transform[3][1] = position.y;
    transform[3][2] = position.z;
  }
}
void Node::scale(const vec3 &scale) { set_scale(get_scale() * scale); }
void Node::translate(const vec3 &translation) {
  set_position(get_position() + translation);
}
void Node::rotate(const glm::quat &rotation) {
  set_rotation(rotation * get_rotation());
}
void Node::set_rotation(const glm::quat &rotation) {
  if (auto parentNode = parent.lock()) {
    glm::quat parentRotation = glm::inverse(parentNode->get_rotation());
    glm::quat localRotation = parentRotation * rotation;
    glm::mat3 rotationMatrix = glm::mat3_cast(localRotation);
    transform[0] = glm::vec4(rotationMatrix[0], transform[0].w);
    transform[1] = glm::vec4(rotationMatrix[1], transform[1].w);
    transform[2] = glm::vec4(rotationMatrix[2], transform[2].w);
  } else {
    glm::mat3 rotationMatrix = glm::mat3_cast(rotation);
    transform[0] = glm::vec4(rotationMatrix[0], transform[0].w);
    transform[1] = glm::vec4(rotationMatrix[1], transform[1].w);
    transform[2] = glm::vec4(rotationMatrix[2], transform[2].w);
  }
}
void Node::set_scale(const vec3 &scale) {
  if (auto parentNode = parent.lock()) {
    vec3 parentScale = parentNode->get_scale();
    vec3 localScale = scale / parentScale;
    transform[0][0] = localScale.x;
    transform[1][1] = localScale.y;
    transform[2][2] = localScale.z;
  } else {
    transform[0][0] = scale.x;
    transform[1][1] = scale.y;
    transform[2][2] = scale.z;
  }
}
void Node::awake() {
  for (auto component : components) {
    if (!component->is_awake)
      component->awake();
  }
}
void Node::update(float dt) {
  for (auto component : components) {
    component->update(dt);
  }
}
void Node::on_collision(const physics::Collision &collision) {
  for (auto &component : components) {
    component->on_collision(collision);
  }
}