#ifndef OBJS_BLOCK_HPP
#define OBJS_BLOCK_HPP

#include <string>
#include <vector>
#include <raylib.h>

// Block

struct Block {
   enum Type { air, grass, dirt, solid, sand, water };
   
   Texture* tex = nullptr;
   Type type = Type::air;
   int id = 0;

   Color& getColor();
};

// Map

struct Map {
   std::vector<std::vector<Block>> blocks;
   int sizeX = 0;
   int sizeY = 0;

   // Set block functions

   void setSize(int x, int y);
   void setBlock(int x, int y, const std::string& name);
   void deleteBlock(int x, int y);
   void moveBlock(int ox, int oy, int nx, int ny);

   // Get block functions

   bool isPositionValid(int x, int y);
   bool is(int x, int y, Block::Type type);

   std::vector<Block>& operator[](size_t index);
};

#endif
