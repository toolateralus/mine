#include "../include/renderer.hpp"
#include "../include/mesh.hpp"
#include "../include/engine.hpp"
#include <assimp/matrix4x4.h>
#include <iostream>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

MeshRenderer::~MeshRenderer() {}
MeshRenderer::MeshRenderer(const shared_ptr<Material> &material,
                           const std::string &mesh_path)
    : material(material), mesh(make_shared<Mesh>(mesh_path)) {
      
    }
    
std::unordered_map<std::string, shared_ptr<Mesh>> Mesh::cache = {};
    
void Mesh::load(const std::string &path) {
  auto it = cache.find(path);
  if (it != cache.end()) {
    vertices = it->second->vertices;
    texcoords = it->second->texcoords;
    normals = it->second->normals;
    indices = it->second->indices;
    return;
  }
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error("ERROR::ASSIMP::" +
                             std::string(importer.GetErrorString()));
  }
  process_node(scene->mRootNode, scene);
  auto unique_mesh = make_shared<Mesh>(*this);
  cache[path] = unique_mesh;
}
void Mesh::process_mesh(aiMesh *mesh, const aiScene *scene) {
  unsigned int vertexOffset = vertices.size() / 3; // Each vertex has 3 components (x, y, z)

  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    aiVector3D vertex = mesh->mVertices[i];
    vertices.push_back(vertex.x / 2.0);
    vertices.push_back(vertex.y / 2.0);
    vertices.push_back(vertex.z / 2.0);
    if (mesh->HasTextureCoords(0)) {
      aiVector3D texcoord = mesh->mTextureCoords[0][i];
      texcoords.push_back(texcoord.x);
      texcoords.push_back(texcoord.y);
    }
    if (mesh->HasNormals()) {
      aiVector3D normal = mesh->mNormals[i];
      normals.push_back(normal.x);
      normals.push_back(normal.y);
      normals.push_back(normal.z);
    }
  }
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j] + vertexOffset);
    }
  }
}
void Mesh::process_node(aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    process_mesh(mesh, scene);
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene);
  }
}



void MeshRenderer::deserialize(const YAML::Node &in) {
  material = make_shared<Material>();
  auto material_node = in["material"];
  material->deserialize(material_node);
  auto mesh_path = in["mesh"].as<std::string>();
  mesh = make_shared<Mesh>(mesh_path);
}
void MeshRenderer::serialize(YAML::Emitter &out) {
  out << YAML::BeginMap;
  out << YAML::Key << "type" << YAML::Value << "MeshRenderer";
  out << YAML::Key << "material" << YAML::Value << material->serialize();
  out << YAML::Key << "mesh" << YAML::Value << mesh->path;
  out << YAML::EndMap;
}

void MeshRenderer::awake() {
  auto &engine = Engine::current();
  engine.m_renderer->mesh_buffer->meshes.push_back(shared_from_this());
}
