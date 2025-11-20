#include "PerlinNoise.hpp"
#include "objs/generation.hpp"
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

void generateMap(Map& map, int mapSizeX, int mapSizeY) {
   map.setSize(mapSizeX, mapSizeY);
   generateTerrain(map);
   generateDebri(map);
   generateWater(map);
}

void generateTerrain(Map& map) {
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

      map.setBlock(x, y, "grass");
      for (int yy = y + 1; yy < map.sizeY; ++yy) {
         map.setBlock(x, yy, (yy - y < rockOffset ? "dirt" : "stone"));
      }
   }
}

void generateWater(Map& map) {
   int seaY = map.sizeY * seaLevel;
   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = seaY; y < map.sizeY and map.is(x, y, Block::air); ++y) {
         map.setBlock(x, y, "water");
      }
   }
}

void generateDebri(Map& map) {
   siv::PerlinNoise sandNoise (rand());
   siv::PerlinNoise dirtNoise (rand());

   for (int x = 0; x < map.sizeX; ++x) {
      for (int y = 0; y < map.sizeY; ++y) {
         if (map.is(x, y, Block::air) or map.is(x, y, Block::grass)) {
            continue;
         }

         float value = normalizedNoise2D(dirtNoise, x, y, 0.04f);
         if (value >= .825f) {
            map.setBlock(x, y, "clay");
         } else if (value <= .2f) {
            map.setBlock(x, y, "dirt");
         } else if (not map.is(x, y, Block::dirt) and normalizedNoise2D(sandNoise, x, y, 0.04f) <= .15f) {
            map.setBlock(x, y, "sand");
         }
      }
   }
}
