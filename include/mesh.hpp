#pragma once
#include "component.hpp"
#include "usings.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/ext/matrix_transform.hpp>
struct Material;


struct Mesh : public std::enable_shared_from_this<Mesh> {
  vector<float> vertices = {};
  vector<float> texcoords = {};
  vector<float> normals = {};
  vector<unsigned int> indices = {};
  vector<shared_ptr<Mesh>> submeshes = {};
  mat4 transform = glm::identity<mat4>();
  std::string path;
  static unordered_map<std::string, shared_ptr<Mesh>> cache;
  Mesh(const std::string &path) : path(path) {
    
  }
  ~Mesh() {}
  static void load_into(shared_ptr<Mesh> &mesh, const std::string &path);

private:
  static void process_node(vector<float> &vertices, vector<float> &texcoords,
                        vector<float> &normals, vector<unsigned int> &indices, const aiNode *node, const aiScene *scene);
  static void process_mesh(vector<float> &vertices, vector<float> &texcoords, vector<float> &normals, vector<unsigned int> &indices, aiMesh *mesh, const aiScene *scene);
};

class MeshRenderer : public Component, public std::enable_shared_from_this<MeshRenderer> {
public:
  shared_ptr<Mesh> mesh;
  shared_ptr<Material> material;
  vec4 color = vec4(1);
  MeshRenderer() = default;
  MeshRenderer(const shared_ptr<Material> &material, const std::string &mesh_path);
  ~MeshRenderer() override;
  void awake() override;
  void update(const float &dt) override {}
  void serialize(YAML::Emitter &out) override;
  void deserialize(const YAML::Node &in) override;
};