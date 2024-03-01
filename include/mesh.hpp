#pragma once
#include "component.hpp"
#include "usings.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stdexcept>
#include <unordered_map>
struct Material;


struct Mesh : public std::enable_shared_from_this<Mesh> {
  vector<float> vertices;
  vector<float> texcoords;
  vector<float> normals;
  vector<unsigned int> indices;
  std::string path;
  static std::unordered_map<std::string, shared_ptr<Mesh>> cache;
  Mesh(const std::string &path) : path(path){
    load(path);
  }
  ~Mesh() {}
  void load(const std::string &path);

private:
  void process_node(aiNode *node, const aiScene *scene);
  void process_mesh(aiMesh *mesh, const aiScene *scene);
};

class MeshRenderer : public Component {
public:
  shared_ptr<Mesh> mesh;
  shared_ptr<Material> material;
  vec4 color = vec4(1);
  MeshRenderer() = default;
  MeshRenderer(const shared_ptr<Material> &material, const shared_ptr<Mesh> &mesh);
  ~MeshRenderer() override;
  void awake() override {}
  void update(const float &dt) override {}
  void serialize(YAML::Emitter &out) override;
  void deserialize(const YAML::Node &in) override;
};