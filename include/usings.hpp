#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/gtx/quaternion.hpp>

#define USING_SMART_POINTERS\
 using std::weak_ptr, std::make_shared, std::shared_ptr, std::vector;
 
#define USING_GLM_TYPES\
 using glm::vec3, glm::vec4, glm::mat4, glm::quat;
 
USING_GLM_TYPES
USING_SMART_POINTERS