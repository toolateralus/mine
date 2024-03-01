#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "usings.hpp"
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include "fileio.hpp"

class Shader {
public:
  GLuint program_id;
  std::unordered_map<std::string, GLuint> uniform_locations = {};
  std::string vertex_path, frag_path;
  Shader() {}
  void compile_shader(const std::string &vertex_path,
                 const std::string &fragment_path);
                 
  Shader(const std::string vertex_path, const std::string frag_path);
  ~Shader();
  YAML::Node serialize();
  void deserialize(const YAML::Node &in);
};