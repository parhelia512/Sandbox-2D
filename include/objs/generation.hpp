#ifndef OBJS_GENERATION_HPP
#define OBJS_GENERATION_HPP

#include "objs/furniture.hpp"

// Generate functions

struct FileMap {
   // Let's assume that Furniture::id_t == Block::id_t to avoid circular dependencies
   std::vector<std::vector<Furniture::id_t>> blocks;
   std::vector<Furniture> furniture;
   int sizeX = 0, sizeY = 0;
};

void generateMap(const std::string& name, int sizeX, int sizeY);
void generateTerrain(FileMap& map);
void generateWater(FileMap& map);
void generateDebri(FileMap& map);
void generateTrees(FileMap& map);

#endif
