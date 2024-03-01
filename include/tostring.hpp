#pragma once

#include <string>
#include "usings.hpp"

static std::string vec3_to_string(const vec3 &vec) {
  return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " +
         std::to_string(vec.z) + ")";
}
static vec3 string_to_vec3(const std::string &str) {
  vec3 vec;
  std::string s = str;
  s.erase(0, 1);
  s.erase(s.size() - 1, 1);
  std::string delimiter = ", ";
  size_t pos = 0;
  std::string token;
  int i = 0;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    vec[i] = std::stof(token);
    s.erase(0, pos + delimiter.length());
    i++;
  }
  vec[i] = std::stof(s);
  return vec;
}
static std::string vec4_to_string(const vec4 &vec) {
  return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " +
         std::to_string(vec.z) + ", " + std::to_string(vec.w) + ")";
}
static vec4 string_to_vec4(const std::string &str) {
  vec4 vec;
  std::string s = str;
  s.erase(0, 1);
  s.erase(s.size() - 1, 1);
  std::string delimiter = ", ";
  size_t pos = 0;
  std::string token;
  int i = 0;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    vec[i] = std::stof(token);
    s.erase(0, pos + delimiter.length());
    i++;
  }
  vec[i] = std::stof(s);
  return vec;
}
static std::string quat_to_string(const quat &q) {
  return "(" + std::to_string(q.x) + ", " + std::to_string(q.y) + ", " +
         std::to_string(q.z) + ", " + std::to_string(q.w) + ")";
}
static quat string_to_quat(const std::string &str) {
  quat q;
  std::string s = str;
  s.erase(0, 1);
  s.erase(s.size() - 1, 1);
  std::string delimiter = ", ";
  size_t pos = 0;
  std::string token;
  int i = 0;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    q[i] = std::stof(token);
    s.erase(0, pos + delimiter.length());
    i++;
  }
  q[i] = std::stof(s);
  return q;
}
static std::string mat4_to_string(const mat4 &m) {
  std::string str = "";
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      str += std::to_string(m[i][j]);
      if (i != 3 || j != 3) {
        str += ", ";
      }
    }
  }
  return str;
}
static mat4 string_to_mat4(const std::string &str) {
  mat4 m;
  std::string s = str;
  s.erase(0, 1);
  s.erase(s.size() - 1, 1);
  std::string delimiter = ", ";
  size_t pos = 0;
  std::string token;
  int i = 0;
  int j = 0;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    token = s.substr(0, pos);
    m[i][j] = std::stof(token);
    s.erase(0, pos + delimiter.length());
    j++;
    if (j == 4) {
      j = 0;
      i++;
    }
  }
  m[i][j] = std::stof(s);
  return m;
}

