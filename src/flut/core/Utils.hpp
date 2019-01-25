#pragma once

#include <vector>
#include <cstdint>
#include <fstream>

namespace flut {
  namespace core {

    class Utils {
    public:
      static std::vector<char> loadFileText(const std::string &filePath) {
        std::ifstream file(filePath, std::ios_base::in | std::ios_base::binary);
        if (!file.is_open())
          return {};
        static std::vector<char> result;
        file.seekg(0, std::ios_base::end);
        result.resize(file.tellg());
        file.seekg(0, std::ios_base::beg);
        file.read(result.data(), result.size());
        return result;
      }
    };
  }
}
