#include "../include/renderer.hpp"
#include "../include/camera.hpp"
#include "../include/engine.hpp"
#include "../include/fileio.hpp"
#include "../include/input.hpp"
#include "../include/mesh.hpp"
#include "../include/light.hpp"

#include "../thirdparty/stb/stb_image.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <unistd.h>

Shader::Shader(const std::string vertex_path, const std::string fragment_path) : vertex_path(vertex_path), frag_path(fragment_path){
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
  
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  
  // get uniform locations
  // add new uniforms here if your shader calls for it.
  static const auto uniforms = std::vector<std::string>{
      "viewProjectionMatrix", "modelMatrix", "color", "lightPosition",
      "lightColor", "lightRadius", "lightIntensity", "castShadows", "hasTexture", "textureSampler"};
  glUseProgram(program_id);
  for (auto i = 0; i < uniforms.size(); i++) {
    const auto uniform_path = uniforms[i].c_str();
    const auto location = glGetUniformLocation(program_id, uniform_path);
    if (location != -1)
      uniform_locations[uniforms[i]] = location;
    else {
      //std::cout << "shader " << fragmentPath << " failed to bind uniform: " << uniforms[i] << std::endl;
    }
  }
}

auto Gizmo::shader = make_shared<Shader>(std::string(Engine::RESOURCE_DIR_PATH + "/shaders/gizmo_vert.hlsl"), std::string(Engine::RESOURCE_DIR_PATH + "/shaders/gizmo_frag.hlsl"));

Shader::~Shader() { glDeleteProgram(program_id); }

// VertexBuffer
void MeshBuffer::update_data() {
  vertices.clear();
  texcoords.clear();
  normals.clear();
  indices.clear();
  interleaved_data.clear();
  
  for (auto mr : this->meshes) {
    auto mesh = mr->mesh;
    vertices.insert(vertices.end(), mesh->vertices.begin(), mesh->vertices.end());
    texcoords.insert(texcoords.end(), mesh->texcoords.begin(), mesh->texcoords.end());
    normals.insert(normals.end(), mesh->normals.begin(), mesh->normals.end());
    indices.insert(indices.end(), mesh->indices.begin(), mesh->indices.end());
  }
  
  // Interleave vertices, texcoords, and normals
  for (size_t i = 0; i < vertices.size() / 3; ++i) {
    interleaved_data.push_back(vertices[i*3]);
    interleaved_data.push_back(vertices[i*3 + 1]);
    interleaved_data.push_back(vertices[i*3 + 2]);
    
    interleaved_data.push_back(texcoords[i*2]);
    interleaved_data.push_back(texcoords[i*2 + 1]);
    
    interleaved_data.push_back(normals[i*3]);
    interleaved_data.push_back(normals[i*3 + 1]);
    interleaved_data.push_back(normals[i*3 + 2]);
  }
  
  // save some memory by dumping this off early
  // probably a negligble performance gain. (if any)
  normals.clear();
  vertices.clear();
  texcoords.clear();
  
  glBindVertexArray(vao);
  
  // Buffer the interleaved data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, interleaved_data.size() * sizeof(float), interleaved_data.data(), GL_STATIC_DRAW);
  
  // Buffer the indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
  indices.clear();
  
  // Set the vertex attributes pointers
  size_t stride = (3 + 2 + 3) * sizeof(float);
  
  // vertex position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
  glEnableVertexAttribArray(0);
  // texture coordinates
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // normals
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
  glEnableVertexAttribArray(2);
  
  glBindVertexArray(0);
}
MeshBuffer::MeshBuffer() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &model_vbo);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
}
MeshBuffer::~MeshBuffer() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  this->meshes.clear();
}
// Renderer
Renderer::Renderer(const char *title, const int h, const int w,
                   void (*update_loop)(const float &dt))
    : update_loop(update_loop), screenWidth(w), screenHeight(h), title(title) {
  init_opengl();
  init_imgui();
  // the vertex buffer can only be instantiated after GL context is initialized.
  mesh_buffer = make_shared<MeshBuffer>();
  gizmo_buffer = make_shared<GizmoBuffer>();
}
void Renderer::init_opengl() {
  glfwInit();
  window = glfwCreateWindow(screenWidth, screenHeight, title, nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glewExperimental = GL_TRUE;
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glfwSetFramebufferSizeCallback(window, resizeCallback);
  glfwSwapInterval(VSYNC_ENABLED);
}
// static
void Renderer::resizeCallback(GLFWwindow *window, int w, int h) {
  glViewport(0, 0, w, h);
}
Renderer::~Renderer() { 
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate(); 
}

/**
 * @brief Runs the main rendering loop.
 *
 * This function continuously updates the scene and physics, renders the scene,
 * and polls for metrics until the window is closed.
 *
 * @return The exit code of the application.
 */
int Renderer::run() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    const auto start = std::chrono::high_resolution_clock::now();
    
    const auto scene = Engine::current().m_scene;
    
    update_loop(dt);
    
    const auto cam = scene->camera->get_component<Camera>();
    
    if (cam == nullptr) {
      throw std::runtime_error("No camera found in scene.");
    }
    
    // TODO: optimize this, we re-scrape all mesh data every frame.
    mesh_buffer->update_data();
    gizmo_buffer->update_data();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(cam->sky_color.x, cam->sky_color.y, cam->sky_color.z, 1.0f);
    
    const auto viewProjectionMatrix = cam->get_view_projection();
    draw_meshes(viewProjectionMatrix);
    draw_gizmos(viewProjectionMatrix);
    draw_imgui();
    
    glfwSwapBuffers(window);
    poll_metrics(start);
  }
  return EXIT_CODE;
}

void Renderer::poll_metrics(
  const std::chrono::time_point<std::chrono::high_resolution_clock> &start) {
  auto endTime = std::chrono::high_resolution_clock::now();
  dt = std::chrono::duration<float>(endTime - start).count();
  framerate = 1 / dt;

  if (Input::current().key_down(Key::Escape)) {
    std::cout << "fps: " << framerate << std::endl;
  }
}

void Renderer::apply_lighting_uniforms(const shared_ptr<Shader> &shader, const GLuint &shader_program) const {
  const auto light_node = Engine::current().m_scene->light;
  const auto light = light_node->get_component<Light>();
  
  const auto light_position = light_node->get_position();
  const auto light_color = light->color;
  const auto light_radius = light->range;
  const auto light_intensity = light->intensity;
  const auto cast_shadows = light->cast_shadows;
  
  const auto lightPositionLocation = shader->uniform_locations.find("lightPosition");
  const auto lightColorLocation = shader->uniform_locations.find("lightColor");
  const auto lightRadiusLocation = shader->uniform_locations.find("lightRadius");
  const auto lightIntensityLocation = shader->uniform_locations.find("lightIntensity");
  const auto castShadowsLocation = shader->uniform_locations.find("castShadows");
    
  if (lightPositionLocation != shader->uniform_locations.end()) {
    glUniform3fv(lightPositionLocation->second, 1, glm::value_ptr(light_position));
  }
  
  if (lightColorLocation != shader->uniform_locations.end()) {
    glUniform3fv(lightColorLocation->second, 1, glm::value_ptr(light_color));
  }
  
  if (lightRadiusLocation != shader->uniform_locations.end()) {
    glUniform1f(lightRadiusLocation->second, light_radius);
  }
  
  if (lightIntensityLocation != shader->uniform_locations.end()) {
    glUniform1f(lightIntensityLocation->second, light_intensity);
  }
  
  if (castShadowsLocation != shader->uniform_locations.end()) {
    glUniform1i(castShadowsLocation->second, cast_shadows);
  }
}
void Renderer::apply_uniforms(
    const mat4 &viewProjectionMatrix,
    std::shared_ptr<MeshRenderer> &mesh_renderer) const {
      
  const auto material = mesh_renderer->material;
  const auto shader_struct = material->shader;
  const auto shader = shader_struct->program_id;
  const auto uniforms = shader_struct->uniform_locations;
  const auto texture = material->texture;
  const auto node = mesh_renderer->node.lock();
  const auto transform_matrix = node->get_transform();
  
  glUseProgram(shader);
  
  // SET TEXTURE UNIFORMS
  {
    auto samplerLoc = uniforms.find("textureSampler");
    const auto hasTextureLoc = uniforms.find("hasTexture");
    
    if (samplerLoc != uniforms.end() && hasTextureLoc != uniforms.end() && texture.has_value()) {
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(samplerLoc->second, 0);
        glUniform1i(hasTextureLoc->second, 1);
        glBindTexture(GL_TEXTURE_2D, texture.value()->texture);
    } else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUniform1i(hasTextureLoc->second, 0);
    }
  }
  // MESH & MATRIX UNIFORMS
  {
    const auto colorLocation = uniforms.find("color");
    const auto viewProjectionMatrixLocation =
        uniforms.find("viewProjectionMatrix");
    const auto modelMatrixLocation = uniforms.find("modelMatrix");
    
    if (colorLocation != uniforms.end())
      glUniform4fv(colorLocation->second, 1,
                  glm::value_ptr(mesh_renderer->color));
    
    if (viewProjectionMatrixLocation != uniforms.end())
      glUniformMatrix4fv(viewProjectionMatrixLocation->second, 1, GL_FALSE,
                        glm::value_ptr(viewProjectionMatrix));

    if (modelMatrixLocation != uniforms.end())
      glUniformMatrix4fv(modelMatrixLocation->second, 1, GL_FALSE,
                        glm::value_ptr(transform_matrix));
  }
  
  apply_lighting_uniforms(mesh_renderer->material->shader, shader);
}
void Renderer::draw_meshes(const mat4 &viewProjectionMatrix) const {
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT, GL_FILL_NV);
  void *indexOffset = 0;
  glBindVertexArray(mesh_buffer->vao);
  for (auto &mesh_renderer : mesh_buffer->meshes) {
    
    const auto indexCount = mesh_renderer->mesh->indices.size();
    
    apply_uniforms(viewProjectionMatrix, mesh_renderer);

    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indexOffset);
    
    indexOffset = (char *)indexOffset + indexCount * sizeof(unsigned int);
  }
}

void Renderer::draw_gizmos(const mat4 &viewProjectionMatrix) const {
  glDisable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT, GL_LINE);
  //std::cout << "drawing " << gizmo_buffer->gizmos.size() << " gizmos" << std::endl;
  glBindVertexArray(gizmo_buffer->VAO);
  const auto shader = Gizmo::shader->program_id;
  glUseProgram(shader);
  
  const auto colorLocation = glGetUniformLocation(shader, "color");
  const auto mmXLocation = glGetUniformLocation(shader, "modelMatrix");
  const auto viewProjectionMatrixLocation = glGetUniformLocation(shader, "viewProjectionMatrix");
  
  void *indexOffset = 0;
  for (auto &gizmo : gizmo_buffer->gizmos) {
    
    const auto indexCount = gizmo.indices.size();
    const auto node = gizmo.node.lock();
    const auto transform_matrix = node->get_transform();
    
    glUniform4fv(colorLocation, 1, glm::value_ptr(gizmo.color));
    
    glUniformMatrix4fv(viewProjectionMatrixLocation, 1, GL_FALSE,
                        glm::value_ptr(viewProjectionMatrix));
    
    glUniformMatrix4fv(mmXLocation, 1, GL_FALSE,
                        glm::value_ptr(transform_matrix));
    
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, indexOffset);
    
    indexOffset = (char *)indexOffset + indexCount * sizeof(unsigned int);
  }
  
  gizmo_buffer->gizmos.clear();
}

Texture::Texture(const std::string path) : path(path) {
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  stbi_set_flip_vertically_on_load(true);
  data = stbi_load(path.c_str(), &width, &height, &channel_count, 0);
  
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cerr << "Failed to load texture: " << path << std::endl;
  }
  
  stbi_image_free(data);
}
Texture::~Texture() { glDeleteTextures(1, &texture); }

shared_ptr<MeshRenderer>
Renderer::add_mesh(shared_ptr<Node> &node, const shared_ptr<Material> &material,
                   const std::string &path) {
  auto mesh = make_shared<Mesh>(path);
  auto mr = node->add_component<MeshRenderer>(material, mesh);
  mesh_buffer->meshes.push_back(mr);
  return mr;
}
void Renderer::add_gizmo(const Gizmo &gizmo) {
  gizmo_buffer->gizmos.push_back(gizmo);
}
void Renderer::init_imgui() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  const char *glsl_version = "#version 130";
  ImGui_ImplOpenGL3_Init(glsl_version);
}
void Renderer::draw_imgui() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  auto &engine = Engine::current();
  engine.on_gui();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
