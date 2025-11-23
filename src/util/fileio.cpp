#include <fstream>
#include <vector>
#include "util/fileio.hpp"
#include "util/random.hpp"

// File functions

std::string getRandomLineFromFile(const std::filesystem::path& path) {
   std::fstream file (path.string().c_str());
   if (not file.is_open()) {
      return "";
   }

   std::vector<std::string> lines;
   std::string line;

   while (std::getline(file, line)) {
      lines.push_back(line);
   }
   return lines[random(0, lines.size() - 1)];
}
