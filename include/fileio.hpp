#pragma once
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>

static const bool file_exists(const std::string &name) {
  std::ifstream f(name.c_str());
  return f.good();
}
static const char *read_file(const std::string filePath) {
  std::ifstream fileStream(filePath,
                           std::ios::in | std::ios::binary | std::ios::ate);
  if (!fileStream.is_open()) {
    std::cerr << "Could not read file " << filePath << ". File does not exist."
              << std::endl;
    return nullptr;
  }
  std::streampos size = fileStream.tellg();
  char *buffer = new char[size + std::streampos(1)];
  fileStream.seekg(0, std::ios::beg);
  fileStream.read(buffer, size);
  buffer[size] = '\0';
  fileStream.close();
  return buffer;
}
static const void write_file(const std::string filePath, const char *data) {
    std::ofstream fileStream(filePath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!fileStream.is_open()) {
        std::cerr << "Could not write file " << filePath << ". File does not exist."
                  << std::endl;
        return;
    }
    fileStream.write(data, strlen(data));
    fileStream.close();
}