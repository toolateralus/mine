#pragma once
#include "mesh.hpp"

#include "usings.hpp"
#include <chrono>
#include <memory>
#include <optional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "node.hpp"

#define IMGUI_HAS_DOCK

#include "shader.hpp"
#include <yaml-cpp/yaml.h>

static int EXIT_CODE = 0;
constexpr int SCREEN_W = 640;
constexpr int SCREEN_H = 480;

static int VSYNC_ENABLED = GL_TRUE;
struct MeshRenderer;


class Texture {
public:
  GLuint texture;
  int width, height, channel_count;
  unsigned char *data;
  std::string path;
  void load_texture(const std::string &path);
  Texture() = default;
  Texture(const std::string path);
  ~Texture();
  YAML::Node serialize();
  void deserialize(const YAML::Node &in);
};

class Material {
public:
  shared_ptr<Shader> shader;
  optional<shared_ptr<Texture>> texture;
  Material(); // This should only be used when deserializing.
  Material(shared_ptr<Shader> shader,
           optional<shared_ptr<Texture>> texture = std::nullopt)
      : shader(shader), texture(texture) {}
  ~Material() {
    this->shader.reset();
    if (this->texture.has_value()) {
      this->texture.value().reset();
    }
  }
  YAML::Node serialize();
  void deserialize(const YAML::Node &in);
};

class MeshBuffer {
public:
  GLuint vbo, vao, ebo;
  vector<unsigned int> indices = {};
  vector<float> interleaved_data = {};
  vector<shared_ptr<MeshRenderer>> meshes = {};
  MeshBuffer();
  ~MeshBuffer();
  void init();
  void update_data();
};

struct Gizmo {
  weak_ptr<Node> node;
  vector<float> vertices;
  vector<unsigned int> indices;
  vec4 color = {1, 1, 1, 1};
  static shared_ptr<Shader> shader;
  static Gizmo create_line(weak_ptr<Node> &owner, const vec3 &start,
                           const vec3 &end, const vec4 &color) {
    Gizmo gizmo(owner);
    auto vertices = {vec4(start, 1.0f), vec4(end, 1.0f)};
    for (const auto &vertex : vertices) {
      gizmo.vertices.push_back(vertex.x);
      gizmo.vertices.push_back(vertex.y);
      gizmo.vertices.push_back(vertex.z);
    }
    gizmo.indices = {0, 1};
    gizmo.color = color;
    return gizmo;
  }
  static Gizmo create_sphere(weak_ptr<Node> &owner, const float radius,
                             const float height, const int segments = 32,
                             const int rings = 32) {
    Gizmo gizmo(owner);
    for (int i = 0; i <= segments; i++) {
      float theta = (float)i / (float)segments * 2.0f * glm::pi<float>();
      for (int j = 0; j <= rings; j++) {
        float phi = (float)j / (float)rings * glm::pi<float>();
        float x = cosf(theta) * sinf(phi);
        float y = cosf(phi);
        float z = sinf(theta) * sinf(phi);
        vec4 vertex = vec4(x * radius, y * radius, z * radius, 1.0f);
        gizmo.vertices.push_back(vertex.x);
        gizmo.vertices.push_back(vertex.y);
        gizmo.vertices.push_back(vertex.z);
      }
    }
    for (int i = 0; i < segments; i++) {
      for (int j = 0; j < rings; j++) {
        gizmo.indices.push_back(i * (rings + 1) + j);
        gizmo.indices.push_back((i + 1) * (rings + 1) + j);
        gizmo.indices.push_back((i + 1) * (rings + 1) + (j + 1));
        gizmo.indices.push_back(i * (rings + 1) + j);
        gizmo.indices.push_back((i + 1) * (rings + 1) + (j + 1));
        gizmo.indices.push_back(i * (rings + 1) + (j + 1));
      }
    }
    return gizmo;
  }
  static Gizmo create_cube(weak_ptr<Node> &owner, const vec3 &size,
                           const vec4 &color) {
    Gizmo gizmo(owner);
    Mesh mesh(std::string("res/prim_mesh/cube.obj"));
    gizmo.vertices = mesh.vertices;
    gizmo.indices = mesh.indices;
    gizmo.color = color;
    return gizmo;
  }
  Gizmo(weak_ptr<Node> &owner) : node(owner) {}
  ~Gizmo() {}
};

class GizmoBuffer {
public:
  GLuint modelVBO, VBO, VAO, EBO;
  vector<float> vertices = {};
  vector<unsigned int> indices = {};
  vector<glm::mat4> modelMatrices = {};
  vector<Gizmo> gizmos = {};
  GizmoBuffer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &modelVBO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
  }
  ~GizmoBuffer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
  }
  void update_data() {
    modelMatrices.clear();
    vertices.clear();
    indices.clear();

    for (auto gizmo : this->gizmos) {
      auto node = gizmo.node.lock();
      modelMatrices.insert(modelMatrices.end(), node->get_transform());
      vertices.insert(vertices.end(), gizmo.vertices.begin(),
                      gizmo.vertices.end());
      indices.insert(indices.end(), gizmo.indices.begin(), gizmo.indices.end());
    }

    glBindVertexArray(VAO);

    // vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    // matrices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (void *)(vertices.size() * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
  }
};

class Renderer {
public:
  GLFWwindow *window;
  shared_ptr<MeshBuffer> mesh_buffer;
  shared_ptr<GizmoBuffer> gizmo_buffer;
  float dt, framerate;
  const char *title;
  int screenWidth;
  int screenHeight;
  bool running = true;
  void (*update_loop)(const float &dt);
  
  Renderer(const char *title, const int h, const int w,
           void (*update_loop)(const float &dt));
  ~Renderer();

  void add_gizmo(const Gizmo &gizmo);
  int run();
  void init_opengl();
  void init_imgui();
  
  void draw_meshes(const mat4 &viewProjectionMatrix) const;
  void draw_gizmos(const mat4 &viewProjectionMatrix) const;
  void draw_imgui();
  
  static void resizeCallback(GLFWwindow *window, int width, int height);
  
  void poll_metrics(const std::chrono::time_point<std::chrono::high_resolution_clock> &start);
      
  void apply_lighting_uniforms(const shared_ptr<Shader> &shader,
                               const GLuint &program_id) const;
                               
  void apply_uniforms(const mat4 &viewProjectionMatrix,
                    std::shared_ptr<MeshRenderer> &mesh_renderer) const;
};
