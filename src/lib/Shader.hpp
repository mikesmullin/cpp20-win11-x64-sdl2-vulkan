#pragma once

#include <string>
#include <vector>

namespace mks {

class Shader {
 public:
  Shader();
  ~Shader();

  std::vector<char> readFile(const std::string& filePath) const;
};
}  // namespace mks