#include <array>
#include <unordered_map>
#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "util/format.hpp" // IWYU pragma: export

// Constants

constexpr int idCount = 13;

static std::unordered_map<std::string, int> blockIds {
   {"air", 0}, {"grass", 1}, {"dirt", 2}, {"clay", 3}, {"stone", 4},
   {"sand", 5}, {"sandstone", 6}, {"water", 7}, {"bricks", 8}, {"glass", 9},
   {"planks", 10}, {"stone_bricks", 11}, {"tiles", 12}
};

static std::array<Block::Type, idCount> blockTypes {{
   Block::air, Block::grass, Block::dirt, Block::solid, Block::solid,
   Block::sand, Block::solid, Block::water, Block::solid, Block::solid,
   Block::solid, Block::solid, Block::solid
}};

static std::array<Color, idCount> blockColors {{
   {0, 0, 0, 0}, {28, 152, 29, 255}, {117, 56, 19, 255}, {158, 91, 35, 255}, {102, 102, 102, 255},
   {255, 189, 40, 255}, {247, 134, 13, 255}, {8, 69, 165, 255}, {244, 52, 8, 255}, {0, 0, 0, 0},
   {158, 91, 35, 255}, {102, 102, 102, 255}, {102, 102, 102, 255}
}};

// Block functions

Color& Block::getColor() {
   return blockColors[id];
}

// Set block functions

void Map::setSize(int x, int y) {
   sizeX = x;
   sizeY = y;
   blocks = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
}

void Map::setBlock(int x, int y, const std::string& name) {
   assert(blockIds.find(name) != blockIds.end(), "Block with the name '{}' does not exist.", name);
   auto& block = blocks[y][x];
   
   block.tex = &ResourceManager::get().getTexture(name);
   block.id = blockIds[name];
   block.type = blockTypes[block.id];
}

void Map::deleteBlock(int x, int y) {
   auto& block = blocks[y][x];
   block.tex = nullptr;
   block.type = Block::air;
   block.id = 0;
}

void Map::moveBlock(int ox, int oy, int nx, int ny) {
   std::swap(blocks[oy][ox], blocks[ny][nx]);
}

// Get block functions

bool Map::isPositionValid(int x, int y) {
   return x >= 0 and x < sizeX and y >= 0 and y < sizeY;
}

bool Map::is(int x, int y, Block::Type type) {
   return isPositionValid(x, y) and blocks[y][x].type == type;
}

std::vector<Block>& Map::operator[](size_t index) {
   return blocks[index];
}
