#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "objs/item.hpp"
#include "objs/player.hpp"
#include "util/format.hpp"
#include "util/parallax.hpp"
#include "util/render.hpp"
#include <array>
#include <cmath>
#include <unordered_map>

// Constants

constexpr unsigned short blockCount = 22;

// NOTE: due to logic in gameState.cpp, any grass blocks must be defined RIGHT
// BEFORE the dirt block, for an example, you can see blocks 1 and 2
static inline const std::unordered_map<std::string, unsigned short> blockIds {
   {"air", 0}, {"grass", 1}, {"dirt", 2}, {"clay", 3}, {"stone", 4},
   {"sand", 5}, {"sandstone", 6}, {"water", 7}, {"bricks", 8}, {"glass", 9},
   {"planks", 10}, {"stone_bricks", 11}, {"tiles", 12}, {"obsidian", 13}, {"lava", 14},
   {"platform", 15}, {"snow", 16}, {"ice", 17}, {"jungle_grass", 18}, {"mud", 19},
   {"lamp", 20}, {"torch", 21}
};

constexpr static inline std::array<const char*, blockCount> blockNames {
   "air", "grass", "dirt", "clay", "stone",
   "sand", "sandstone", "water", "bricks", "glass",
   "planks", "stone_bricks", "tiles", "obsidian", "lava",
   "platform", "snow", "ice", "jungle_grass", "mud",
   "lamp", "torch"
};

// This is a nightmare to edit, but at least makes other code easier!
constexpr static inline const std::array<BlockType, blockCount> blockAttributes {{
   BlockType::empty | BlockType::transparent, BlockType::solid | BlockType::grass, BlockType::solid | BlockType::dirt, BlockType::solid, BlockType::solid,
   BlockType::solid | BlockType::sand, BlockType::solid, BlockType::transparent | BlockType::water | BlockType::liquid, BlockType::solid, BlockType::solid | BlockType::transparent,
   BlockType::solid, BlockType::solid, BlockType::solid, BlockType::solid, BlockType::transparent | BlockType::lava | BlockType::lightsource | BlockType::liquid,
   BlockType::platform | BlockType::transparent | BlockType::solid, BlockType::sand | BlockType::solid, BlockType::ice | BlockType::solid, BlockType::solid | BlockType::grass, BlockType::solid | BlockType::dirt,
   BlockType::lightsource | BlockType::solid, BlockType::transparent | BlockType::lightsource | BlockType::torch,
}};

// Block getter functions

// Asserts in these two functions would be too slow, as they're called often (especially in
// world generation code), that's why debug asserts are used instead
unsigned short getBlockIdFromName(const std::string &name) {
   assertDebug(blockIds.count(name), "DEBUG: Block with the name '{}' does not exist!", name);
   return blockIds.at(name);
}

std::string getBlockNameFromId(unsigned short id) {
   assertDebug(id < blockCount, "DEBUG: Block with the id '{}' does not exist. Valid IDs are in range {} to {}.", (int)id, 0, (int)blockCount - 1);
   return blockNames.at(id);
}

// Map constructors

void Map::init() {
   timeShaderLocation = GetShaderLocation(getShader("water"), "time");
   lightmap = LoadRenderTexture(GetScreenWidth() / 2, GetScreenHeight() / 2);

   blocks = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
   walls  = std::vector<std::vector<Block>>(sizeY, std::vector<Block>(sizeX, Block{}));
}

Map::~Map() {
   UnloadRenderTexture(lightmap);
}

// Set block functions

void Map::setRow(int y, const std::string &name, bool isWall) {
   unsigned short id = getBlockIdFromName(name);
   BlockType type = blockAttributes[id];

   (isWall ? walls : blocks)[y] = std::vector<Block>(sizeX, Block{&getTexture(name), type, id, ((type & BlockType::liquid) ? maxWaterLayers : (unsigned char)0)});
}

void Map::setRow(int y, unsigned short *ids, unsigned char *physicsValues) {
   for (int x = 0; x < sizeX; ++x) {
      Block &block = blocks[y][x];
      block.id = ids[x];
      block.value = 0;
      block.value2 = physicsValues[x];
      block.type = blockAttributes[block.id];

      if (block.id != 0) {
         block.texture = &getTexture(blockNames[block.id]);
      }
   }
}

void Map::setWallRow(int y, unsigned short *ids) {
   for (int x = 0; x < sizeX; ++x) {
      Block &block = walls[y][x];
      block.id = ids[x];
      block.value = block.value2 = 0;
      block.type = blockAttributes[block.id];

      if (block.id != 0) {
         block.texture = &getTexture(blockNames[block.id]);
      }
   }
}

void Map::setBlock(int x, int y, const std::string &name, bool isWall) {
   Block &block = (isWall ? walls : blocks)[y][x];
   
   block.id = getBlockIdFromName(name);
   block.value = block.value2 = 0;
   block.type = blockAttributes[block.id] | (block.type % (BlockType::furniture | BlockType::furnitureTop));

   if (block.id != 0) {
      block.texture = &getTexture(name);
   }

   if (block.type & BlockType::liquid) {
      block.value2 = maxWaterLayers;
   }
}

void Map::setBlock(int x, int y, unsigned short id, bool isWall) {
   setBlock(x, y, getBlockNameFromId(id), isWall);
}

// Fuck furniture logic, rewrite it one day
void Map::deleteBlock(int x, int y, bool wall) {
   Block &block = (wall ? walls : blocks)[y][x];
   block.texture = nullptr;

   block.type = blockAttributes[0] | (block.type % (BlockType::furniture | BlockType::furnitureTop));
   block.id = block.value = block.value2 = 0;
}

// Fuck this
void Map::moveBlock(int oldX, int oldY, int newX, int newY) {
   constexpr BlockType furnitureMask = (BlockType::furniture | BlockType::furnitureTop);
   BlockType oldType = blocks[oldY][oldX].type % furnitureMask;
   BlockType newType = blocks[newY][newX].type % furnitureMask;

   std::swap(blocks[oldY][oldX], blocks[newY][newX]);

   blocks[newY][newX].type = (blocks[newY][newX].type % ~furnitureMask) | oldType;
   blocks[oldY][oldX].type = (blocks[oldY][oldX].type % ~furnitureMask) | newType;
}

// Set furniture functions

void Map::addFurniture(Furniture &object) {
   for (int y = object.posY; y < object.sizeY + object.posY; ++y) {
      for (int x = object.posX; x < object.sizeX + object.posX; ++x) {
         if (object.pieces[y - object.posY][x - object.posX].nil) {
            continue;
         }
         blocks[y][x].type = blocks[y][x].type | BlockType::furniture;
         if (object.isWalkable && y == object.posY) {
            blocks[y][x].type = blocks[y][x].type | BlockType::furnitureTop;
         }
      }
   }
   furniture.push_back(object);
}

void Map::removeFurniture(Furniture &object) {
   for (int y = object.posY; y < object.sizeY + object.posY; ++y) {
      for (int x = object.posX; x < object.sizeX + object.posX; ++x) {
         if (!object.pieces[y - object.posY][x - object.posX].nil) {
            // Again, modulus is overloaded as 'bitwise and' here.
            blocks[y][x].type = blocks[y][x].type % ~(BlockType::furniture | BlockType::furnitureTop);
         }
      }
   }
   object.deleted = true;
}

// Get block functions

bool Map::isPositionValid(int x, int y) const {
   return x >= 0 && x < sizeX && y >= 0 && y < sizeY;
}

bool Map::is(int x, int y, BlockType type) const {
   return isPositionValid(x, y) && (blocks[y][x].type & type);
}

bool Map::isu(int x, int y, BlockType type) const {
   return blocks[y][x].type & type;
}

bool Map::isSoil(int x, int y) const {
   return isPositionValid(x, y) && (blocks[y][x].type & (BlockType::grass | BlockType::dirt | BlockType::sand));
}

bool Map::isLiquid(int x, int y) const {
   return isPositionValid(x, y) && (blocks[y][x].type & BlockType::liquid) && blocks[y][x].value2 > minWaterLayers;
}

bool Map::isEmpty(int x, int y) const {
   return isPositionValid(x, y) && ((blocks[y][x].type & BlockType::empty) || ((blocks[y][x].type & BlockType::liquid) && blocks[y][x].value <= minWaterLayers));
}

bool Map::isNotSolid(int x, int y) const {
   return isPositionValid(x, y) && (blocks[y][x].type & (BlockType::empty | BlockType::liquid)) && !(blocks[y][x].type & BlockType::furniture);
}

bool Map::isStable(int x, int y) const {
   return isPositionValid(x, y) && (blocks[y][x].type & (BlockType::solid | BlockType::furniture)) && !(blocks[y][x].type & BlockType::platform);
}

bool Map::isPlatformedFurniture(int x, int y) const {
   return (blocks[y][x].type & BlockType::furniture) && (blocks[y][x].type & BlockType::furnitureTop);
}

// Render functions

void Map::renderLight(const Camera2D &camera, Texture2D &texture, float x, float y, const Vector2 &size, const Color &color) const {
   drawTexture(texture, {(((x + 0.5f - camera.target.x) * camera.zoom) + camera.offset.x) / 2.0f, (((y + 0.5f - camera.target.y) * camera.zoom) + camera.offset.y) / 2.0f}, size, 0, color);
}

void Map::render(const std::vector<DroppedItem> &droppedItems, const Player &player, float accumulator, const Rectangle &cameraBounds, const Camera2D &camera) const {
   // Render background walls
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         const Block &wall = walls[y][x];
         if ((wall.type & BlockType::empty) || !(blocks[y][x].type & BlockType::transparent)) {
            continue;
         }

         int oldX = x;
         while (x <= cameraBounds.width && walls[y][x].id == wall.id && (blocks[y][x].type & BlockType::transparent)) {
            x += 1;
         }

         drawTextureBlock(*wall.texture, {(float)oldX, (float)y, float(x - oldX), 1}, wallTint);
         x -= 1;
      }
   }

   // Render furniture
   for (const Furniture &obj: furniture) {
      obj.render(cameraBounds);
   }

   // Render furniture-like blocks before the player
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         const Block &block = blocks[y][x];
         if (block.type & BlockType::platform) {
            DrawTexturePro(*block.texture, {0, 0, (float)block.texture->width, (float)block.texture->height}, {(float)x, (float)y, 1, 1}, {0, 0}, 0, WHITE);
         } else if (block.type & BlockType::torch) {
            constexpr static float torchLightOffsetsY[] = {-1.0f, -1.0f * (5.0f / 8.0f), -0.75f, -0.75f, -1.0f * (5.0f / 8.0f)};

            float textureSize = block.texture->height / 2.0f;
            DrawTexturePro(*block.texture, {textureSize * block.value2, 0, textureSize, textureSize}, {(float)x, (float)y, 1, 1}, {0, 0}, 0, WHITE);
            DrawTexturePro(*block.texture, {textureSize * block.value, textureSize, textureSize, textureSize}, {(float)x, (float)y + torchLightOffsetsY[block.value2], 1, 1}, {0, 0}, 0, WHITE);
         }
      }
   }

   // Render the player
   player.render(accumulator);

   for (const DroppedItem &droppedItem : droppedItems) {
      droppedItem.render();
   }

   Shader &waterShader = getShader("water");
   float time = GetTime();
   SetShaderValue(waterShader, timeShaderLocation, &time, SHADER_UNIFORM_FLOAT);

   struct Liquid {
      const Block *pointer;
      int x, y;
   };
   static std::vector<Liquid> liquidTiles;
   liquidTiles.clear();

   // Render blocks
   for (int y = cameraBounds.y; y <= cameraBounds.height; ++y) {
      for (int x = cameraBounds.x; x <= cameraBounds.width; ++x) {
         const Block &block = blocks[y][x];
         if ((block.type & BlockType::empty) || (block.type & BlockType::torch)) {
            continue;
         }

         // Render fluids
         if ((block.type & BlockType::liquid) || (block.type & BlockType::platform)) {
            if (block.value2 > minWaterLayers) {
               liquidTiles.push_back(Liquid{&block, x, y});
            }
            continue;
         }

         // Render regular blocks
         int oldX = x;
         while (x <= cameraBounds.width && blocks[y][x].id == block.id) {
            x += 1;
         }

         drawTextureBlock(*block.texture, {(float)oldX, (float)y, float(x - oldX), 1});
         x -= 1;
      }
   }

   // Render fluids
   BeginShaderMode(waterShader);
   for (const Liquid &block: liquidTiles) {
      float height = (float)block.pointer->value2 / (float)maxWaterLayers;
      Color liquidFlags;
      liquidFlags.r = (is(block.x, block.y - 1, BlockType::solid) && !is(block.x, block.y - 1, BlockType::platform) ? 255 : 0);
      liquidFlags.g = (!is(block.x, block.y + 1, block.pointer->type) ? 255 : 0);

      drawFluidBlock(*block.pointer->texture, {(float)block.x, (float)block.y + (1 - height), 1, height}, Fade(liquidFlags, height));
   }
   EndShaderMode();

   // // Render lights
   BeginTextureMode(lightmap);
   ClearBackground(BLACK);
   BeginBlendMode(BLEND_ADDITIVE);

   int lightBoundsMinX = std::max<int>(0, cameraBounds.x - 8);
   int lightBoundsMinY = std::max<int>(0, cameraBounds.y - 8);
   int lightBoundsMaxX = std::min<int>(sizeX - 1, cameraBounds.width + 8);
   int lightBoundsMaxY = std::min<int>(sizeY - 1, cameraBounds.height + 8);

   Color airLightColor   = getLightBasedOnTime();
   Color waterLightColor = Fade(airLightColor, 0.1f);

   // const function hack
   static float counter = 0.0f;
   counter += GetFrameTime();

   float sizeOffset     = std::sin(counter * 1.5f) * camera.zoom * 0.4f;
   float positionOffset = std::cos(counter * 0.8f) * camera.zoom * 0.0075f;

   Vector2 lightSize      = {3.5f * camera.zoom, 3.5f * camera.zoom};
   Vector2 lightLargeSize = {lightSize.x + lightSize.x, lightSize.y + lightSize.y};
   Vector2 lightHugeSize  = {lightLargeSize.x + lightSize.x, lightLargeSize.y + lightSize.y};
   Vector2 liquidSize     = {lightSize.x + sizeOffset, lightSize.y + sizeOffset};

   Texture2D &lightHugeTexture  = getTexture("lightsource_6x");
   Texture2D &lightLargeTexture = getTexture("lightsource_4x");
   Texture2D &lightTexture      = getTexture("lightsource_2x");

   for (int y = lightBoundsMinY; y <= lightBoundsMaxY; ++y) {
      for (int x = lightBoundsMinX; x <= lightBoundsMaxX; ++x) {
         BlockType type = blocks[y][x].type;
         if (!(type & BlockType::lightsource) && !(type & BlockType::transparent)) {
            continue;
         }

         // Direct light sources, ones who do not require the wall behind to be transparent
         if (type & BlockType::lava) {
            if (blocks[y][x].value2 <= minWaterLayers) {
               continue;
            }
            renderLight(camera, lightTexture, x + positionOffset, y + positionOffset, liquidSize, {255, 125, 0, 255});
         } else if (type & BlockType::torch) {
            renderLight(camera, lightHugeTexture, x + positionOffset, y + positionOffset, lightHugeSize, {255, 200, 160, 255}); // Light orange
         } else if (type & BlockType::lightsource) {
            renderLight(camera, lightLargeTexture, x, y, lightLargeSize, {255, 255, 0, 255});
         }

         if (!(walls[y][x].type & BlockType::transparent)) {
            continue;
         }

         // Indirect light sources, these require to background to be empty
         if ((type & BlockType::water) && blocks[y][x].value2 > minWaterLayers) {
            renderLight(camera, lightTexture, x, y, lightSize, waterLightColor);
         } else {
            renderLight(camera, lightTexture, x, y, lightSize, airLightColor);
         }
      }
   }

   EndBlendMode();
   EndTextureMode();

   BeginBlendMode(BLEND_MULTIPLIED);
   DrawTexturePro(lightmap.texture, {0, 0, (float)lightmap.texture.width, -(float)lightmap.texture.height}, {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()}, {0, 0}, 0, WHITE);
   EndBlendMode();

   BeginMode2D(camera); // EndTextureMode disables it for some reason
}
