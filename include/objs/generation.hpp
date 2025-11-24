#ifndef OBJS_GENERATION_HPP
#define OBJS_GENERATION_HPP

#include "objs/map.hpp"

// Generate functions

using FileMap = std::vector<std::vector<Block::id_t>>;

void generateMap(const std::string& name, int sizeX, int sizeY);
void generateTerrain(FileMap& map, int sizeX, int sizeY);
void generateWater(FileMap& map, int sizeX, int sizeY);
void generateDebri(FileMap& map, int sizeX, int sizeY);

#endif
