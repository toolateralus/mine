#pragma once
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

#define USING_STD_COMMON \
using std::string, std::vector, std::unordered_map, std::array, std::optional, std::cout, std::cin;

#define USING_SMART_POINTERS\
 using std::string, std::weak_ptr, std::make_shared, std::shared_ptr;
 
#define USING_GLM_TYPES \
 using namespace glm; \
 using glm::vec3, glm::vec4, glm::mat4, glm::quat, glm::vec2; \
 
 
USING_STD_COMMON
USING_GLM_TYPES
USING_SMART_POINTERS