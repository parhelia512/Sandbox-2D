#ifndef OBJS_GENERATION_HPP
#define OBJS_GENERATION_HPP

#include "objs/map.hpp"

// Generate functions

struct FileMap {
   std::vector<std::vector<Block::id_t>> blocks;
   int sizeX = 0, sizeY = 0;
};

void generateMap(const std::string& name, int sizeX, int sizeY);
void generateTerrain(FileMap& map);
void generateWater(FileMap& map);
void generateDebri(FileMap& map);
void generateTrees(FileMap& map);

#endif
