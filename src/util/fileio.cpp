#include <fstream>
#include <vector>
#include "util/fileio.hpp"
#include "util/format.hpp"
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

// World saving functions

void save(std::ofstream& file, float playerX, float playerY, float zoom, int sizeX, int sizeY) {
   file << playerX << '\n' << playerY << '\n';
   file << sizeX << '\n' << sizeY << '\n';
   file << zoom << '\n';
}

void saveWorldData(const std::string& name, float playerX, float playerY, float zoom, const Map& map) {
   std::ofstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to save world 'data/worlds/{}.txt'.", name);
   save(file, playerX, playerY, zoom, map.sizeX, map.sizeY);

   for (const auto& row: map.blocks) {
      for (const auto& tile: row) {
         file << (int)tile.id << ' ';
      }
      file << '\n';
   }

   for (const auto& row: map.walls) {
      for (const auto& tile: row) {
         file << (int)tile.id << ' ';
      }
      file << '\n';
   }
   file.close();
}

void saveWorldData(const std::string& name, float playerX, float playerY, float zoom, const FileMap& map) {
   std::ofstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to save world 'data/worlds/{}.txt'.", name);
   save(file, playerX, playerY, zoom, map.sizeX, map.sizeY);

   for (const auto& row: map.blocks) {
      for (const auto& tile: row) {
         file << (int)tile << ' ';
      }
      file << '\n';
   }

   // Don't forget to dump background walls too
   for (const auto& row: map.blocks) {
      for (const auto& tile: row) {
         file << 0 << ' ';
      }
      file << '\n';
   }
   file.close();
}

// World loading functions

void loadWorldData(const std::string& name, Player& player, float& zoom, Map& map) {
   std::ifstream file (format("data/worlds/{}.txt", name));
   assert(file.is_open(), "Failed to load world 'data/worlds/{}.txt'.", name);

   file >> player.pos.x >> player.pos.y;
   file >> map.sizeX >> map.sizeY;
   file >> zoom;
   map.init();

   for (int y = 0; y < map.sizeY; ++y) {
      for (int x = 0; x < map.sizeX; ++x) {
         int id = 0;
         file >> id;
         map.setBlock(x, y, (Block::id_t)id);
      }
   }

   // Do the same for background walls
   for (int y = 0; y < map.sizeY; ++y) {
      for (int x = 0; x < map.sizeX; ++x) {
         int id = 0;
         file >> id;
         map.setBlock(x, y, (Block::id_t)id, true);
      }
   }
   player.init();
}
