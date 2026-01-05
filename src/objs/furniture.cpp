#include "objs/furniture.hpp"
#include "mngr/resource.hpp"
#include "objs/map.hpp"
#include "util/random.hpp"
#include <array>
#include <unordered_map>

// Constants

constexpr int cactusGrowSpeedMin  = 350;
constexpr int cactusGrowSpeedMax  = 1750;
constexpr int saplingGrowSpeedMin = 200;
constexpr int saplingGrowSpeedMax = 1500;

constexpr int cactusSizeMin = 4;
constexpr int cactusSizeMax = 9;
constexpr int palmSizeMin   = 8;
constexpr int palmSizeMax   = 22;
constexpr int treeSizeMin   = 5;
constexpr int treeSizeMax   = 18;

constexpr int treeRootChance     = 25;
constexpr int treeBranchChance   = 15;
constexpr int cactusBranchChance = 50;
constexpr int cactusFlowerChance = 10;

constexpr int textureSize    = 8;
constexpr int furnitureCount = 11;

static inline const std::unordered_map<std::string, int> furnitureTextureIds {
   {"tree", 0}, {"sapling", 1}, {"palm", 2}, {"palm_sapling", 3}, {"pine", 4},
   {"pine_sapling", 5}, {"jungle_tree", 6}, {"jungle_sapling", 7}, {"cactus", 8}, {"cactus_seed", 9},
   {"table", 10}
};

static inline const std::array<const char*, furnitureCount> furnitureTextureNames {
   "tree", "sapling", "palm", "palm_sapling", "pine",
   "pine_sapling", "jungle_tree", "jungle_sapling", "cactus", "cactus_seed",
   "table"
};

// Helper functions

inline void setBlock(FurniturePiece &piece, int tx, int ty) {
   piece.nil = false;
   piece.tx = tx;
   piece.ty = ty;
}

// Furniture functions

Furniture::Furniture(Type type, unsigned char texId, int value, int value2, int posX, int posY, int sizeX, int sizeY)
   : type(type), texId(texId), value(value), value2(value2), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));   
}

Furniture::Furniture(const std::string &texture, int posX, int posY, int sizeX, int sizeY, Type type)
   : type(type), texId(furnitureTextureIds.at(texture)), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));
}

// Update furniture

void Furniture::update(Map &map) {
   switch (type) {

   // Update trees

   case Type::tree: {
      if (!map.isSoil(posX + 1, posY + sizeY)) {
         map.removeFurniture(*this);
         return;
      }
   } break;

   // Update saplings

   case Type::sapling: {
      // Either there's no soil below the sapling or there's sand on top of it
      if (!map.isSoil(posX, posY + sizeY) || map.is(posX, posY - 1, BlockType::sand)) {
         map.removeFurniture(*this);
         return;
      }

      // The sapling is covered in liquid
      if (map.isLiquid(posX, posY) && map.isLiquid(posX, posY + 1)) {
         map.removeFurniture(*this);
         return;
      }

      if (value == 0) {
         value2 = random(saplingGrowSpeedMin, saplingGrowSpeedMax);
      }

      value += 1;
      if (value >= value2) {
         // Our beautiful tree has grown
         map.removeFurniture(*this);
         Furniture::generate(posX, posY + 1, map, Type::tree);
      }
   } break;

   // Update cacti

   case Type::cactus: {
      // Tile below it is not sand
      if (map.blocks[posY + sizeY][posX + 1].id != getBlockIdFromName("sand")) {
         map.removeFurniture(*this);
         return;
      }
   } break;

   // Update cacti seeds

   case Type::cactus_seed: {
      // Tile below it is not sand or tile above it is sand (crushed) or it is covered in liquid
      if (map.blocks[posY + 1][posX].id != getBlockIdFromName("sand") || map.is(posX, posY - 1, BlockType::sand) || map.isLiquid(posX, posY)) {
         map.removeFurniture(*this);
         return;
      }

      if (value == 0) {
         value2 = random(cactusGrowSpeedMin, cactusGrowSpeedMax);
      }

      value += 1;
      if (value >= value2) {
         // Our beautiful cactus has grown
         map.removeFurniture(*this);
         Furniture::generate(posX, posY, map, Type::cactus);
      }
   } break;

   // Update tables (yes, they also need this)

   case Type::table: {
      // Any of the tiles below it isn't solid
      if (!map.is(posX, posY + sizeY, BlockType::solid) || !map.is(posX + 1, posY + sizeY, BlockType::solid) || !map.is(posX + 2, posY + sizeY, BlockType::solid)) {
         map.removeFurniture(*this);
         return;
      }
   } break;

   default: break;
   }
}

// Get functions

Furniture Furniture::get(int x, int y, const Map &map, Type type, bool debug) {
   switch (type) {

   // Make a tree

   case Type::tree: {
      if (!debug && (x < 1 || x >= map.sizeX - 1 || y < 0 || !map.isSoil(x , y + 1))) {
         return {};
      }
      
      bool palm = (map.isu(x, y + 1, BlockType::sand) && map.blocks[y + 1][x].id == getBlockIdFromName("sand"));
      int height = (palm ? random(palmSizeMin, palmSizeMax) : random(treeSizeMin, treeSizeMax));
      for (int i = 0; i < height && i < map.sizeY; ++i) {
         if (!map.isNotSolid(x, y - i)) {
            height = i;
            break;
         }
      }

      if (!debug && (height < (palm ? palmSizeMin : treeSizeMin))) {
         return {};
      }
      static std::unordered_map<unsigned char, std::string> textureMap {
         {getBlockIdFromName("grass"), "tree"}, {getBlockIdFromName("dirt"), "tree"},        {getBlockIdFromName("sand"),         "palm"},
         {getBlockIdFromName("snow"),  "pine"}, {getBlockIdFromName("mud"),  "jungle_tree"}, {getBlockIdFromName("jungle_grass"), "jungle_tree"}
      };

      Furniture tree (textureMap[map.blocks[y + 1][x].id], x - 1, y - height + 1, 3, height, Furniture::tree);
      int offsetTx = (chance(50) ? 0 : 3 * textureSize);

      for (int i = 0; i < (palm ? 3 : 2); ++i) {
         for (int j = 0; j < 3; ++j) {
            setBlock(tree.pieces[i][j], offsetTx + j * textureSize, i * textureSize);
         }
      }

      for (int i = (palm ? 3 : 2); i < height; ++i) {
         FurniturePiece &piece = tree.pieces[i][1];
         if (palm) {
            setBlock(piece, random(0, 2) * textureSize, (i + 1 == height ? 4 : 3) * textureSize);
            continue;
         }

         setBlock(piece, 2 * textureSize, 3 * textureSize);

         if (i + 1 == height) {
            bool rightFree = (map.isNotSolid(tree.posX + 2, tree.posY + i) && map.isSoil(tree.posX + 2, tree.posY + i + 1) && chance(treeRootChance));
            bool leftFree = (map.isNotSolid(tree.posX, tree.posY + i) && map.isSoil(tree.posX, tree.posY + i + 1) && chance(treeRootChance));

            if (rightFree) {
               setBlock(tree.pieces[i][2], 4 * textureSize, 4 * textureSize);
               piece.tx = 3 * textureSize;
               piece.ty = 4 * textureSize;
            }
            if (leftFree) {
               setBlock(tree.pieces[i][0], 0 * textureSize, 4 * textureSize);
               piece.tx = 1 * textureSize;
               piece.ty = 4 * textureSize;
            }
            if (rightFree && leftFree) {
               piece.tx = 5 * textureSize;
               piece.ty = 4 * textureSize;
            } else if (!rightFree && !leftFree) {
               piece.ty = 4 * textureSize;
            }
         } else {
            bool rightFree = (map.isNotSolid(tree.posX + 2, tree.posY + i) && chance(treeBranchChance));
            bool leftFree = (map.isNotSolid(tree.posX, tree.posY + i) && chance(treeBranchChance));

            if (rightFree) {
               setBlock(tree.pieces[i][2], random(3, 5) * textureSize, 2 * textureSize);
               piece.tx = 3 * textureSize;
               piece.ty = 3 * textureSize;
            }
            if (leftFree) {
               setBlock(tree.pieces[i][0], random(0, 2) * textureSize, 2 * textureSize);
               piece.tx = 1 * textureSize;
               piece.ty = 3 * textureSize;
            }
            if (rightFree && leftFree) {
               piece.tx = 5 * textureSize;
               piece.ty = 3 * textureSize;
            }
         }
      }
      return tree;
   } break;

   // Make a sapling

   case Type::sapling: {
      if (!debug && (x < 0 || x >= map.sizeX || y < 0 || !map.isSoil(x , y + 2) || !map.isEmpty(x, y) || !map.isEmpty(x, y + 1))) {
         return {};
      }

      static std::unordered_map<unsigned char, std::string> textureMap {
         {getBlockIdFromName("grass"), "sapling"},      {getBlockIdFromName("dirt"), "sapling"},        {getBlockIdFromName("sand"),         "palm_sapling"},
         {getBlockIdFromName("snow"),  "pine_sapling"}, {getBlockIdFromName("mud"),  "jungle_sapling"}, {getBlockIdFromName("jungle_grass"), "jungle_sapling"}
      };
      unsigned char btype = map.blocks[y + 2][x].id;
      Furniture sapling ((!textureMap.count(btype) ? "sapling" : textureMap[btype]), x, y, 1, 2, Furniture::sapling);

      int value = random(0, 100);
      int offsetTx = (value < 33 ? 0 : (value < 66 ? textureSize : 2 * textureSize));

      setBlock(sapling.pieces[0][0], offsetTx, 0 * textureSize);
      setBlock(sapling.pieces[1][0], offsetTx, 1 * textureSize);
      return sapling;
   } break;

   // Make a cactus

   case Type::cactus: {
      if (!debug && (x < 1 || x >= map.sizeX - 1 || y < 0 || y + 1 >= map.sizeY || map.blocks[y + 1][x].id != getBlockIdFromName("sand"))) {
         return {};
      }

      int height = random(cactusSizeMin, cactusSizeMax);
      for (int i = 0; i < height && i < map.sizeY; ++i) {
         if (!map.isNotSolid(x, y - i)) {
            height = i;
            break;
         }
      }

      if (!debug && height <= 0) {
         return {};
      }

      Furniture cactus ("cactus", x - 1, y - height + 1, 3, height, Furniture::cactus);
      cactus.value = height;
      
      for (int i = 0; i < cactus.sizeY; ++i) {
         for (int j = 0; j < cactus.sizeX; ++j) {
            if (!map.isNotSolid(cactus.posX + j, cactus.posY + i)) {
               continue;
            }
            
            if (j == 1) {
               cactus.pieces[i][j].nil = false;
            } else if (i != cactus.sizeY - 1 && i != 0) {
               cactus.pieces[i][j].nil = chance(cactusBranchChance);
            }
         }
      }

      for (int i = 0; i < cactus.sizeY; ++i) {
         for (int j = 0; j < cactus.sizeX; ++j) {
            FurniturePiece &piece = cactus.pieces[i][j];
            
            if (j == 1) {
               bool rightStub = (!cactus.pieces[i][2].nil && i < cactus.sizeY + 1 && cactus.pieces[i + 1][2].nil);
               bool leftStub = (!cactus.pieces[i][0].nil && i < cactus.sizeY + 1 && cactus.pieces[i + 1][0].nil);

               if (rightStub && leftStub) {
                  setBlock(piece, 0 * textureSize, 3 * textureSize);
               } else if (rightStub) {
                  setBlock(piece, 0 * textureSize, 1 * textureSize);
               } else if (leftStub) {
                  setBlock(piece, 0 * textureSize, 2 * textureSize);
               } else {
                  if (i == 0) {
                     setBlock(piece, 1 * textureSize, (chance(cactusFlowerChance) ? 1 : 0) * textureSize);
                  } else if (i == cactus.sizeY - 1) {
                     setBlock(piece, 1 * textureSize, 3 * textureSize);
                  } else {
                     setBlock(piece, 0 * textureSize, 0 * textureSize);
                  }
               }
               continue;
            } else if (!piece.nil) {
               int offsetX = (j == 0 ? 2 : 3) * textureSize;

               if (cactus.pieces[i - 1][j].nil && cactus.pieces[i + 1][j].nil) {
                  setBlock(piece, offsetX, 3 * textureSize);
               } else if (cactus.pieces[i - 1][j].nil) {
                  setBlock(piece, offsetX, 0 * textureSize);
               } else if (cactus.pieces[i + 1][j].nil) {
                  setBlock(piece, offsetX, 2 * textureSize);
               } else {
                  setBlock(piece, offsetX, 1 * textureSize);
               }
            }
         }
      }
      return cactus;
   } break;

   // Make a cactus seed

   case Type::cactus_seed: {
      if (!debug && (y + 1 >= map.sizeY || map.blocks[y + 1][x].id != getBlockIdFromName("sand") || !map.isEmpty(x, y))) {
         return {};
      }

      Furniture cactus_seed ("cactus_seed", x, y, 1, 1, Furniture::cactus_seed);
      int value = random(0, 100);
      int offsetTx = (value < 33 ? 0 : (value < 66 ? textureSize : 2 * textureSize));

      setBlock(cactus_seed.pieces[0][0], offsetTx * textureSize, 0 * textureSize);
      return cactus_seed;
   } break;

   // Make a table

   case Type::table: {
      if (!debug && (!map.is(x, y + 2, BlockType::solid) || !map.is(x + 1, y + 2, BlockType::solid) || !map.is(x + 2, y + 2, BlockType::solid))) {
         return {};
      }

      Furniture table ("table", x, y, 3, 2, Furniture::table);
      table.isWalkable = true;
      
      for (int yy = 0; yy < 2; ++yy) {
         for (int xx = 0; xx < 3; ++xx) {
            if (!debug && !map.isNotSolid(xx + x, yy + y)) {
               return {};
            }
            setBlock(table.pieces[yy][xx], textureSize * xx, textureSize * yy);
         }
      }
      return table;
   } break;

   default: return {};
   };
}

void Furniture::generate(int x, int y, Map &map, Type type) {
   Furniture furniture = Furniture::get(x, y, map, type);
   if (furniture.type != Furniture::none) {
      map.addFurniture(furniture);
   }
}

// Render furniture

void Furniture::preview(const Map &map) const {
   for (int y = posY; y - posY < sizeY; ++y) {
      for (int x = posX; x - posX < sizeX; ++x) {
         const FurniturePiece &piece = pieces[y - posY][x - posX];
         if (piece.nil) {
            continue;
         }
         Color color = Fade((map.isNotSolid(x, y) ? WHITE : RED), previewAlpha);
         DrawTexturePro(getTexture(furnitureTextureNames[texId]), {(float)piece.tx, (float)piece.ty, textureSize, textureSize}, {(float)x, (float)y, 1.f, 1.f}, {0, 0}, 0, color);
      }
   }
}

void Furniture::render(const Rectangle &cameraBounds) const {
   for (int y = posY; y <= cameraBounds.height && y - posY < sizeY; ++y) {
      for (int x = posX; x <= cameraBounds.width && x - posX < sizeX; ++x) {
         const FurniturePiece &piece = pieces[y - posY][x - posX];
         if (y < cameraBounds.y || x < cameraBounds.x || piece.nil) {
            continue;
         }
         DrawTexturePro(getTexture(furnitureTextureNames[texId]), {(float)piece.tx, (float)piece.ty, textureSize, textureSize}, {(float)x, (float)y, 1.f, 1.f}, {0, 0}, 0, WHITE);
      }
   }
}

// Id functions

unsigned char Furniture::getId(const std::string &name) {
   return furnitureTextureIds.at(name);
}

std::string Furniture::getName(unsigned char id) {
   return furnitureTextureNames.at(id);
}

FurnitureTexture Furniture::getFurnitureIcon(unsigned char id) {
   constexpr std::array<Vector2, furnitureCount> textureSizes {{
      {}, {textureSize, textureSize * 2}, {}, {}, {},
      {}, {}, {}, {}, {textureSize, textureSize},
      {textureSize * 3, textureSize * 2},
   }};
   return {getTexture(getName(id)), textureSizes[id].x, textureSizes[id].y};
}
