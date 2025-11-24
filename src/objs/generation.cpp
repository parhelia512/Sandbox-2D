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

// Generate functions

void generateMap(const std::string& name, int sizeX, int sizeY) {
   auto map = FileMap(sizeY, std::vector<Block::id_t>(sizeX, 0));
   generateTerrain(map, sizeX, sizeY);
   generateDebri(map, sizeX, sizeY);
   generateWater(map, sizeX, sizeY);
   saveWorldData(name, sizeX / 2.f, 0.f, 50.f, map);
}

void generateTerrain(FileMap& map, int sizeX, int sizeY) {
   siv::PerlinNoise noise (rand());
   int y = startY * sizeY;
   int rockOffset = rockOffsetStart;

   for (int x = 0; x < sizeX; ++x) {
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

      if (y < sizeY * highThreshold) {
         ++y;
         ++rockOffset;
      }

      if (y > sizeY * lowThreshold) {
         --y;
         --rockOffset;
      }
      y = std::clamp<int>(y, sizeY * highestPoint, sizeY * lowestPoint);
      rockOffset = std::clamp(rockOffset, rockOffsetMin, rockOffsetMax);

      // Generate grass, dirt and stone

      map[y][x] = Block::getId("grass");
      for (int yy = y + 1; yy < sizeY; ++yy) {
         map[yy][x] = Block::getId((yy - y < rockOffset ? "dirt" : "stone"));
      }
   }
}

void generateWater(FileMap& map, int sizeX, int sizeY) {
   int seaY = sizeY * seaLevel;
   for (int x = 0; x < sizeX; ++x) {
      for (int y = seaY; y < sizeY and map[y][x] == Block::getId("air"); ++y) {
         map[y][x] = Block::getId("water");
      }
   }
}

void generateDebri(FileMap& map, int sizeX, int sizeY) {
   siv::PerlinNoise sandNoise (rand());
   siv::PerlinNoise dirtNoise (rand());

   for (int x = 0; x < sizeX; ++x) {
      for (int y = 0; y < sizeY; ++y) {
         if (map[y][x] == Block::getId("air") or map[y][x] == Block::getId("grass")) {
            continue;
         }

         float value = normalizedNoise2D(dirtNoise, x, y, 0.04f);
         if (value >= .825f) {
            map[y][x] = Block::getId("clay");
         } else if (value <= .2f) {
            map[y][x] = Block::getId("dirt");
         } else if (map[y][x] != Block::getId("dirt") and normalizedNoise2D(sandNoise, x, y, 0.04f) <= .15f) {
            map[y][x] = Block::getId("sand");
         }
      }
   }
}
