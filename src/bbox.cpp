#include "../include/bbox.hpp"
#include "../include/tostring.hpp"
using namespace physics;

BoundingBox BoundingBox::merge(const BoundingBox &other) const {
  vec3 mergedMin = glm::min(min, other.min);
  vec3 mergedMax = glm::max(max, other.max);
  return BoundingBox(mergedMin, mergedMax);
}
void BoundingBox::expand(const vec3 &point) {
  min = glm::min(min, point);
  max = glm::max(max, point);
}
std::string BoundingBox::to_string() const {
  return "min: " + vec3_to_string(min) + " max: " + vec3_to_string(max);
}
vec3 BoundingBox::get_center() const { return (min + max) * 0.5f; }
vec3 BoundingBox::get_size() const { return max - min; }
bool BoundingBox::contains(const vec3 &point) const {
  return point.x >= min.x && point.x <= max.x && point.y >= min.y &&
         point.y <= max.y && point.z >= min.z && point.z <= max.z;
}
bool BoundingBox::intersects(const BoundingBox &other) const {
  return min.x <= other.max.x && max.x >= other.min.x && min.y <= other.max.y &&
         max.y >= other.min.y && min.z <= other.max.z && max.z >= other.min.z;
}