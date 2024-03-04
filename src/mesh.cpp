#include "../include/mesh.hpp"
#include "../include/engine.hpp"
#include "../include/renderer.hpp"
#include <assimp/matrix4x4.h>
#include <iostream>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

MeshRenderer::~MeshRenderer() {}

// TODO: implement instanced rendering and use unqiue mesh/material buffers
// this will allow us to avoid redundant data and reduce the number of draw calls.

MeshRenderer::MeshRenderer(const shared_ptr<Material> &material,
                           const std::string &mesh_path)
    : material(material) {
  if (Mesh::cache.find(mesh_path) == Mesh::cache.end()) {
    mesh = make_shared<Mesh>(mesh_path);
    Mesh::load_into(mesh, mesh_path);
  } else {
    mesh = Mesh::cache[mesh_path];
  }
}

unordered_map<std::string, shared_ptr<Mesh>> Mesh::cache = {};

void Mesh::load_into(shared_ptr<Mesh> &mesh, const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error("ERROR::ASSIMP::" +
                             std::string(importer.GetErrorString()));
  }
  Mesh::process_node(mesh, scene->mRootNode, scene);
  cache[path] = mesh;
}
void Mesh::process_mesh(shared_ptr<Mesh> &parent, aiMesh *mesh, const aiScene *scene) {
  auto &vertices = parent->vertices;
  auto &texcoords = parent->texcoords;
  auto &normals = parent->normals;
  auto &indices = parent->indices;
  
  for (size_t i = 0; i < mesh->mNumVertices; i++) {
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
  for (size_t i = 0; i < mesh->mNumFaces; i++) {
    aiFace &face = mesh->mFaces[i];
    for (size_t j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }
}
void Mesh::process_node(shared_ptr<Mesh> &parent, const aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    Mesh::process_mesh(parent, mesh, scene);
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    auto new_mesh = make_shared<Mesh>("");
    parent->submeshes.push_back(new_mesh);
    new_mesh->transform = glm::make_mat4(&node->mTransformation.a1);
    Mesh::process_node(new_mesh, node->mChildren[i], scene);
  }
}
void MeshRenderer::deserialize(const YAML::Node &in) {
  material = make_shared<Material>();
  auto material_node = in["material"];
  material->deserialize(material_node);
  auto mesh_path = in["mesh"].as<std::string>();
  mesh = make_shared<Mesh>(mesh_path);
  Mesh::load_into(mesh, mesh_path);
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
  auto &mesh_buffer = engine.m_renderer->mesh_buffer;
  auto &meshes = mesh_buffer->meshes;
  auto self = shared_from_this();
  auto it = std::find(meshes.begin(), meshes.end(), self);
  auto exists = it != meshes.end();
  
  if (!exists) {
    meshes.push_back(self);
    mesh_buffer->update_data();
  }
}
