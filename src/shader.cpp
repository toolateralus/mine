#include "../include/shader.hpp"

YAML::Node Shader::serialize() {
  YAML::Node out;
  out["vertex_path"] = this->vertex_path;
  out["frag_path"] = this->frag_path;
  return out;
}
void Shader::deserialize(const YAML::Node &in) {
  vertex_path = in["vertex_path"].as<std::string>();
  frag_path = in["frag_path"].as<std::string>();
  compile_shader(vertex_path, frag_path);
}

void Shader::compile_shader(const std::string &vertex_path,
                       const std::string &fragment_path) {
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  auto *vertexSource = read_file(vertex_path);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  
  delete[] vertexSource;
  
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  auto *fragmentSource = read_file(fragment_path);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  
  delete[] fragmentSource;
  
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  
  program_id = glCreateProgram();
  glAttachShader(program_id, vertexShader);
  glAttachShader(program_id, fragmentShader);
  glLinkProgram(program_id);
  
  glGetProgramiv(program_id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program_id, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }
  
  
  
  // get uniform locations
  // add new uniforms here if your shader calls for it.
  static const auto uniforms = std::vector<std::string>{
      "viewProjectionMatrix", "modelMatrix", "color",
      "lightPosition",        "lightColor",  "lightRadius",
      "lightIntensity",       "castShadows", "hasTexture",
      "textureSampler"};
  glUseProgram(program_id);
  for (auto i = 0; i < uniforms.size(); i++) {
    const auto uniform_path = uniforms[i].c_str();
    const auto location = glGetUniformLocation(program_id, uniform_path);
    if (location != -1)
      uniform_locations[uniforms[i]] = location;
    else {
      std::cout << "shader : " << fragment_path << "uniform : " << uniform_path << "not found" << std::endl;
    }
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}
Shader::Shader(const std::string vertex_path, const std::string fragment_path)
    : vertex_path(vertex_path), frag_path(fragment_path) {
  compile_shader(vertex_path, fragment_path);
}
Shader::~Shader() { glDeleteProgram(program_id); }