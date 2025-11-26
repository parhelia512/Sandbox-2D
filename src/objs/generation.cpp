#include "PerlinNoise.hpp"
#include "objs/generation.hpp"
#include "util/fileio.hpp"
#include "util/random.hpp"

// Constants

constexpr float startY = .5f;
constexpr float seaLevel = .4f;
constexpr float highThreshold = .25f;
constexpr float lowThreshold = .5f;
constexpr float highestPoint = .2f;
constexpr float lowestPoint = .55f;

constexpr int rockOffsetStart = 12;
constexpr int rockOffsetMin = 5;
constexpr int rockOffsetMax = 25;

// Private functions

inline float normalizedNoise2D(siv::PerlinNoise& noise, int x, int y, float amplitude) {
   return (noise.octave2D(x * amplitude, y * amplitude, 4) + 1.f) / 2.f;
}

inline float normalizedNoise1D(siv::PerlinNoise& noise, int x, float amplitude) {
   return (noise.octave1D(x * amplitude, 4) + 1.f) / 2.f;
}

// Generate functions

void generateMap(const std::string& name, int sizeX, int sizeY) {
   FileMap map {std::vector<std::vector<Block::id_t>>(sizeY, std::vector<Block::id_t>(sizeX, 0)), sizeX, sizeY};
   generateTerrain(map);
   generateDebri(map);
   generateWater(map);
   generateTrees(map);
   saveWorldData(name, sizeX / 2.f, 0.f, 50.f, map);
}

void generateTerrain(FileMap& map) {
   siv::PerlinNoise noise (rand());
   int y = startY * map.sizeY;
   int rockOffset = rockOffsetStart;

   for (int x = 0; x < map.sizeX; ++x) {
      float value = normalizedNoise2D(noise, x, y, 0.01f);

      // Get different height increase/decrease based on the noise

      if (value < .2f) {
         y += random(-3, 0);
         rockOffset += random(-2, 0);
      } else if (value < .4f) {
         y += random(-2, 0);
         rockOffset += random(-1, 0);
      } else if (value < .65f) {
         y += (chance(50) ? random(-1, 1) : 0);
         rockOffset += (chance(20) ? random(-1, 1) : 0);
      } else if (value < .85f) {
         y += random(0, 2);
         rockOffset += random(0, 1);
      } else {
         y += random(0, 3);
         rockOffset += random(0, 2);
      }

      // Don't let the height get too low or too high

      if (y < map.sizeY * highThreshold) {
         ++y;
         ++rockOffset;
      }

      if (y > map.sizeY * lowThreshold) {
         --y;
         --rockOffset;
      }
      y = std::clamp<int>(y, map.sizeY * highestPoint, map.sizeY * lowestPoint);
      rockOffset = std::clamp(rockOffset, rockOffsetMin, rockOffsetMax);

      // Generate grass, dirt and stone

      map.blocks[y][x] = Block::getId("grass");
      for (int yy = y + 1; yy < map.sizeY; ++yy) {
         map.blocks[yy][x] = Block::getId((yy - y < rockOffset ? "dirt" : "stone"));
      }
   }
}

void generateWater(FileMap& map) {
   int seaY = map.sizeY * seaLevel;
   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = seaY; y < map.sizeY and map.blocks[y][x] == Block::getId("air"); ++y) {
         map.blocks[y][x] = Block::getId("water");
      }
   }
}

void generateDebri(FileMap& map) {
   siv::PerlinNoise sandNoise (rand());
   siv::PerlinNoise dirtNoise (rand());

   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = 0; y < map.sizeY; ++y) {
         if (map.blocks[y][x] == Block::getId("air") or map.blocks[y][x] == Block::getId("grass")) {
            continue;
         }

         float value = normalizedNoise2D(dirtNoise, x, y, 0.04f);
         if (value >= .825f) {
            map.blocks[y][x] = Block::getId("clay");
         } else if (value <= .2f) {
            map.blocks[y][x] = Block::getId("dirt");
         } else if (map.blocks[y][x] != Block::getId("dirt") and normalizedNoise2D(sandNoise, x, y, 0.04f) <= .15f) {
            map.blocks[y][x] = Block::getId("sand");
         }
      }
   }
}

void generateTrees(FileMap& map) {
   siv::PerlinNoise treeNoise (rand());
   float y = startY * map.sizeY + 1;
   int counter = 0;
   
   for (int x = 0; x < map.sizeX; ++x) {
      while (y < map.sizeY - 1 and map.blocks[y + 1][x] == 0) { y++; }
      while (y > 0 and map.blocks[y][x] != 0) { y--; }

      if (map.blocks[y + 1][x] != Block::getId("water") and map.blocks[y + 1][x] != Block::getId("leaf") and normalizedNoise1D(treeNoise, x, 0.04f) >= .65f and counter >= 2) {
         int height = random(6, 11);
         for (int i = 0; i <= height; ++i) {
            map.blocks[y - i][x] = Block::getId("log");
         }

         int radius = random(3, 6);
         int halfR = radius / 2;
         for (int i = 0; i < radius; ++i) {
            for (int j = 0; j < radius; ++j) {
               auto& block = map.blocks[y + i - height - radius + 1][x + j - halfR];
               if (block == 0) {
                  block = Block::getId("leaf");
               }
            }
         }

         int radius2 = radius - 2;
         int halfR2 = radius2 / 2;
         int tHeight = random(1, 2);
         for (int i = 0; i < tHeight; ++i) {
            for (int j = 0; j < radius2; ++j) {
               auto& block = map.blocks[y + i - height - radius - tHeight + 1][x + j - halfR2];
               if (block == 0) {
                  block = Block::getId("leaf");
               }
            }
         }

         counter = 0;
      } else {
         counter++;
      }
   }
}
