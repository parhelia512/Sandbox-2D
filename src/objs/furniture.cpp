#include <array>
#include <unordered_map>
#include "objs/furniture.hpp"
#include "mngr/resource.hpp"
#include "objs/generation.hpp"
#include "objs/map.hpp"
#include "util/random.hpp"

// Constants

constexpr int furnitureCount = 1;
static std::unordered_map<std::string, int> furnitureTextureIds {
   {"tree", 0}
};

constexpr std::array<const char*, furnitureCount> furnitureTextureNames {
   "tree"
};

constexpr int texSize = 8;

// Helper functions

void setBlock(FurniturePiece& piece, const std::string& id, int tx, int ty) {
   piece.nil = false;
   piece.colorId = Block::getId(id);
   piece.tx = tx;
   piece.ty = ty;
}

bool isBlockEmpty(Block::id_t block) {
   return block == 0;
}

bool isBlockSoil(Block::id_t block) {
   return block == Block::getId("grass") or block == Block::getId("dirt");
}

// Furniture functions

Furniture::Furniture(Type type, id_t texId, unsigned char value, unsigned char value2, int posX, int posY, int sizeX, int sizeY)
   : type(type), texId(texId), value(value), value2(value2), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));   
}

Furniture::Furniture(const std::string& texture, int posX, int posY, int sizeX, int sizeY, Type type)
   : texId(furnitureTextureIds[texture]), posX(posX), posY(posY), sizeX(sizeX), sizeY(sizeY), type(type) {
   pieces = std::vector<std::vector<FurniturePiece>>(sizeY, std::vector<FurniturePiece>(sizeX, FurniturePiece{}));
}

void Furniture::render(bool zoomedOut, int minX, int minY, int maxX, int maxY) {
   for (int y = posY; y < maxY and y - posY < sizeY; ++y) {
      for (int x = posX; x < maxX and x - posX < sizeX; ++x) {
         if (y < minY or x < minX) {
            continue;
         }
         
         auto& piece = pieces[y - posY][x - posX];
         if (piece.nil) {
            continue;
         }

         if (zoomedOut) {
            DrawRectangle(x, y, 1, 1, Block::getColorFromId(piece.colorId));
         } else {
            DrawTexturePro(getTexture(furnitureTextureNames[texId]), {(float)piece.tx, (float)piece.ty, texSize, texSize}, {(float)x, (float)y, 1.f, 1.f}, {0, 0}, 0, WHITE);
         }
      }
   }
}

// Get functions

Furniture::id_t Furniture::getId(const std::string& name) {
   return furnitureTextureIds[name];
}

std::string Furniture::getName(id_t id) {
   return furnitureTextureNames[id];
}

// Furniture methods

Furniture generateTree(int x, int y, FileMap& map) {
   if (x < 1 or x >= map.sizeX - 1 or y < 0 or not isBlockSoil(map.blocks[y + 1][x])) {
      return {};
   }
   
   int height = random(5, 18);
   for (int i = 0; i < height and i < map.sizeY; ++i) {
      if (not isBlockEmpty(map.blocks[y - i][x])) {
         height = i + 1;
         break;
      }
   }

   if (height < 5) {
      return {};
   }
   Furniture tree ("tree", x - 1, y - height + 1, 3, height, Furniture::tree);
   int offsetTx = (chance(50) ? 0 : 3 * texSize);

   for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 3; ++j) {
         setBlock(tree.pieces[i][j], "grass", offsetTx + j * texSize, i * texSize);
      }
   }

   for (int i = 2; i < height; ++i) {
      auto& piece = tree.pieces[i][1];
      setBlock(piece, "planks", 2 * texSize, 3 * texSize);

      if (i + 1 == height) {
         bool rightFree = (isBlockEmpty(map.blocks[tree.posY + i][tree.posX + 2]) and isBlockSoil(map.blocks[tree.posY + i + 1][tree.posX + 2]) and chance(25));
         bool leftFree = (isBlockEmpty(map.blocks[tree.posY + i][tree.posX]) and isBlockSoil(map.blocks[tree.posY + i + 1][tree.posX]) and chance(25));

         if (rightFree) {
            setBlock(tree.pieces[i][2], "planks", 4 * texSize, 4 * texSize);
            piece.tx = 3 * texSize;
            piece.ty = 4 * texSize;
         }
         if (leftFree) {
            setBlock(tree.pieces[i][0], "planks", 0 * texSize, 4 * texSize);
            piece.tx = 1 * texSize;
            piece.ty = 4 * texSize;
         }
         if (rightFree and leftFree) {
            piece.tx = 5 * texSize;
            piece.ty = 4 * texSize;
         } else if (not rightFree and not leftFree) {
            piece.ty = 4 * texSize;
         }
      } else {
         bool rightFree = (isBlockEmpty(map.blocks[tree.posY + i][tree.posX + 2]) and chance(10));
         bool leftFree = (isBlockEmpty(map.blocks[tree.posY + i][tree.posX]) and chance(10));

         if (rightFree) {
            setBlock(tree.pieces[i][2], "planks", 4 * texSize, 3 * texSize);
            piece.tx = 3 * texSize;
            piece.tx = 3 * texSize;
         }
         if (leftFree) {
            setBlock(tree.pieces[i][0], "planks", 0 * texSize, 3 * texSize);
            piece.tx = 1 * texSize;
            piece.ty = 3 * texSize;
         }
         if (rightFree and leftFree) {
            piece.tx = 5 * texSize;
            piece.ty = 3 * texSize;
         }
      }
   }
   return tree;
}
