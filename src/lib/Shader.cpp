#include "Shader.hpp"

#include <fstream>
#include <string>
#include <vector>

#include "Logger.hpp"

namespace mks {

Shader::Shader() {
}

Shader::~Shader() {
}

std::vector<char> Shader::readFile(const std::string& filePath) const {
  std::ifstream file{filePath, std::ios::ate | std::ios::binary};

  if (!file.is_open()) {
    throw Logger::Errorf("failed to open file: %s", filePath.c_str());
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  Logger::Debugf("read file: %s", filePath.c_str());
  file.close();
  return buffer;
}

}  // namespace mks