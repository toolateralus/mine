#include "../include/renderer.hpp"
#include "../include/camera.hpp"
#include "../include/engine.hpp"
#include "../include/light.hpp"
#include "../include/mesh.hpp"
#include "../thirdparty/imgui/imgui.h"
#include "../thirdparty/imgui/imgui_impl_glfw.h"
#include "../thirdparty/imgui/imgui_impl_opengl3.h"
#include "../thirdparty/stb/stb_image.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <vector>

auto Gizmo::shader = make_shared<Shader>(
    std::string(Engine::RESOURCE_DIR_PATH + "/shaders/gizmo_vert.glsl"),
    std::string(Engine::RESOURCE_DIR_PATH + "/shaders/gizmo_frag.glsl"));

void MeshBuffer::interleave_mesh(const shared_ptr<Mesh> &mesh) {
  const size_t vertexCount = interleaved_data.size() / 8;
  for (auto &index : mesh->indices) {
    indices.push_back(index + vertexCount);
  }

  const auto mesh_vert_count = mesh->vertices.size() / 3;
  for (size_t i = 0; i < mesh_vert_count; ++i) {
    interleaved_data.push_back(mesh->vertices[i * 3]);
    interleaved_data.push_back(mesh->vertices[i * 3 + 1]);
    interleaved_data.push_back(mesh->vertices[i * 3 + 2]);
    interleaved_data.push_back(mesh->texcoords[i * 2]);
    interleaved_data.push_back(mesh->texcoords[i * 2 + 1]);
    interleaved_data.push_back(mesh->normals[i * 3]);
    interleaved_data.push_back(mesh->normals[i * 3 + 1]);
    interleaved_data.push_back(mesh->normals[i * 3 + 2]);
  }
}

// VertexBuffer
void MeshBuffer::update_data() {
  // indices.clear();
  // interleaved_data.clear();

  // for (const auto &mr : this->meshes) {
  //   interleave_mesh(mr->mesh);
  // }

  glBindVertexArray(vao);

  // Buffer the interleaved data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, interleaved_data.size() * sizeof(float),
               interleaved_data.data(), GL_STATIC_DRAW);

  // Buffer the indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

MeshBuffer::MeshBuffer() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  init();
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
  mesh_buffer = new MeshBuffer();
  gizmo_buffer = new GizmoBuffer();
}
void Renderer::init_opengl() {
  glfwInit();
  window = glfwCreateWindow(screenWidth, screenHeight, title, nullptr, nullptr);
  glfwMakeContextCurrent(window);
  glewExperimental = GL_TRUE;
  glewInit();
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_STATIC_DRAW);
  glfwSetFramebufferSizeCallback(window, resizeCallback);
  glfwSwapInterval(0); // unlimit framerate.
}

void Renderer::resizeCallback(GLFWwindow *window, int w, int h) {
  glViewport(0, 0, w, h);
}

Renderer::~Renderer() {
  delete mesh_buffer;
  delete gizmo_buffer;

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

    const auto &scene = Engine::current().m_scene;

    update_loop(dt);

    if (scene.camera == nullptr) {
      cout << "No camera found in scene." << std::endl;
      continue;
    }

    const auto cam = scene.camera->get_component<Camera>();

    if (cam == nullptr) {
      cout << "No camera component found on camera node." << std::endl;
      continue;
    }

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
}

void Renderer::apply_lighting_uniforms(const shared_ptr<Shader> &shader,
                                       const GLuint &shader_program) {
  const auto light_node = Engine::current().m_scene.light;

  if (!light_node) {
    cout << "no light in scene." << std::endl;
    return;
  }

  const auto light = light_node->get_component<Light>();
  const auto light_position = light_node->get_position();
  const auto light_color = light->color;
  const auto light_radius = light->range;
  const auto light_intensity = light->intensity;
  const auto cast_shadows = light->cast_shadows;

  const auto lightPositionLocation =
      shader->uniform_locations.find("lightPosition");
  const auto lightColorLocation = shader->uniform_locations.find("lightColor");
  const auto lightRadiusLocation =
      shader->uniform_locations.find("lightRadius");
  const auto lightIntensityLocation =
      shader->uniform_locations.find("lightIntensity");
  const auto castShadowsLocation =
      shader->uniform_locations.find("castShadows");

  if (lightPositionLocation != shader->uniform_locations.end()) {
    glUniform3fv(lightPositionLocation->second, 1,
                 glm::value_ptr(light_position));
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
    const std::shared_ptr<MeshRenderer> &mesh_renderer) {

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

    if (samplerLoc != uniforms.end() && hasTextureLoc != uniforms.end() &&
        texture.has_value()) {
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
  mesh_buffer->render(viewProjectionMatrix);
}

void Renderer::draw_gizmos(const mat4 &viewProjectionMatrix) const {
  gizmo_buffer->render(viewProjectionMatrix);
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
  const char *glsl_version = "#version 330";
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

Material::Material() {}

YAML::Node Material::serialize() {
  YAML::Node out;
  out["shader"] = shader->serialize();
  if (this->texture.has_value()) {
    out["texture"] = this->texture.value()->serialize();
  }
  return out;
}
void Material::deserialize(const YAML::Node &in) {
  auto &shader_node = in["shader"];
  this->shader = make_shared<Shader>();
  this->shader->deserialize(shader_node);
  if (in["texture"]) {
    this->texture = make_shared<Texture>();
    this->texture.value()->deserialize(in["texture"]);
  }
}

YAML::Node Texture::serialize() {
  YAML::Node out;
  out["path"] = this->path;
  return out;
}

void Texture::deserialize(const YAML::Node &in) {
  path = in["path"].as<std::string>();
  load_texture(path);
}
void Texture::load_texture(const std::string &path) {
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
Texture::Texture(const std::string path) : path(path) { load_texture(path); }
Texture::~Texture() { glDeleteTextures(1, &texture); }

void MeshBuffer::init() {
  // Set the vertex attributes pointers
  const size_t stride = (3 + 2 + 3) * sizeof(float);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

  // vertex position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
  glEnableVertexAttribArray(0);
  // texture coordinates
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // normals
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *)(5 * sizeof(float)));
  glEnableVertexAttribArray(2);
}
void MeshBuffer::render(const mat4 &viewProjectionMatrix) const {
  glEnable(GL_DEPTH_TEST);
  glPolygonMode(GL_FRONT, GL_FILL_NV);
  size_t indexOffset = 0;
  glBindVertexArray(vao);
  for (const auto &mesh_renderer : meshes) {
    const auto &indexCount = mesh_renderer->mesh->indices.size();
    Renderer::apply_uniforms(viewProjectionMatrix, mesh_renderer);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT,
                   (const void *)indexOffset);
    indexOffset = indexOffset + indexCount * sizeof(unsigned int);
  }
}

void MeshBuffer::erase_mesh(const MeshRenderer *mesh) {
  // shared_ptr<MeshRenderer> renderer;
  
  // int idx = -1;
  // for (size_t i= 0; i < meshes.size();++i) {
  //   if (meshes[i].get() == mesh) {
  //     renderer = meshes[i];
  //     idx = i;
  //     break;
  //   }
  // }
  
  // if (!renderer || idx == -1) return;
  
  // auto it = std::ranges::find(meshes, renderer);
  // meshes.erase(it);
  // erase_interleaved_data(idx, renderer->mesh);
  // refresh();  
}

void MeshBuffer::erase_interleaved_data(const size_t index,
                                        const shared_ptr<Mesh> &mesh) {
  const size_t vertexCount = interleaved_data.size() / 8;
  const size_t mesh_vert_count = mesh->vertices.size() / 3;
  const size_t start_pos = index * 8 * mesh_vert_count;

  // Erase interleaved data
  interleaved_data.erase(interleaved_data.begin() + start_pos,
                         interleaved_data.begin() + start_pos +
                             8 * mesh_vert_count);

  // Adjust indices
  auto it = std::remove_if(indices.begin(), indices.end(),
                           [index, vertexCount, mesh_vert_count](size_t i) {
                             return i >= index * vertexCount &&
                                    i < (index + 1) * vertexCount;
                           });
  indices.erase(it, indices.end());
}
void MeshBuffer::refresh() {
  interleaved_data.clear();
  for (const auto &mr : meshes)
    interleave_mesh(mr->mesh);
}
