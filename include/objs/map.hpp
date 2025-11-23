#ifndef OBJS_BLOCK_HPP
#define OBJS_BLOCK_HPP

#include <string>
#include <vector>
#include <raylib.h>

// Block

struct Block {
   enum Type { air, grass, dirt, solid, transparent, sand, water, lava };
   using id_t = unsigned char;
   
   Texture* tex = nullptr;
   Type type = Type::air;

   // Unsigned chars can only hold 256 unique IDs. Currently trying to save
   // block space, so it's a problem for later
   id_t id = 0;

   // Values used by physics updates, specific to the block type
   unsigned char value = 0;
   unsigned char value2 = 0;

   Color& getColor();
   Color& getWallColor();
   static void initializeColors();
   static int getId(const std::string& name);
};

// Map

struct Map {
   std::vector<std::vector<Block>> blocks;
   std::vector<std::vector<Block>> walls;
   int sizeX = 0;
   int sizeY = 0;

   // Set block functions

   void setSize(int x, int y);
   void setBlock(int x, int y, const std::string& name, bool walls = false);
   void setBlock(int x, int y, Block::id_t id, bool walls = false);
   void deleteBlock(int x, int y, bool walls = false);
   void moveBlock(int ox, int oy, int nx, int ny);

   // Get block functions

   bool isPositionValid(int x, int y);
   bool is(int x, int y, Block::Type type);
   bool isTransparent(int x, int y);

   std::vector<Block>& operator[](size_t index);

   // Render map

   void render(Camera2D& camera);
};

#endif
