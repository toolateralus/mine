#include "../include/mesh.hpp"
#include <assimp/matrix4x4.h>
#include <iostream>
#include <stdexcept>

MeshRenderer::~MeshRenderer() {}
MeshRenderer::MeshRenderer(const shared_ptr<Material> &material,
                           const shared_ptr<Mesh> &mesh)
    : material(material), mesh(mesh) {}
    
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
  processNode(scene->mRootNode, scene);
  auto unique_mesh = make_shared<Mesh>(*this);
  cache[path] = unique_mesh;
}
void Mesh::processMesh(aiMesh *mesh, const aiScene *scene) {
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
      indices.push_back(face.mIndices[j]);
    }
  }
}
void Mesh::processNode(aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    processMesh(mesh, scene);
  }
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}
