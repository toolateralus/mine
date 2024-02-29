#pragma once

#include <string>
#include "usings.hpp"

static std::string vec3_to_string(const vec3 &vec) {
  return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " +
         std::to_string(vec.z) + ")";
}