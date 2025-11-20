#include <algorithm>
#include <cmath>
#include "game/gameState.hpp"
#include "mngr/resource.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include "util/render.hpp"

// Constants

constexpr int mapSizeX = 1250;
constexpr int mapSizeY = 500;

constexpr float cameraFollowSpeed = 17.5f;
constexpr float maxCameraZoom = 1.f;
constexpr float minCameraZoom = 100.f;

// Constructors

GameState::GameState() {
   generateMap(map, mapSizeX, mapSizeY);
   player.init({mapSizeX / 2.f, 0.f});

   camera.target = player.getCenter();
   camera.offset = getScreenCenter();
   camera.rotation = 0.0f;
   camera.zoom = 50.f;
}

// Update functions

void GameState::update() {
   updateControls();
   updatePhysics();
}

void GameState::updateControls() {
   player.updatePlayer(map);
   camera.target = lerp(camera.target, player.getCenter(), 25.f * GetFrameTime());

   float wheel = GetMouseWheelMove();
   if (wheel != 0.f) {
      camera.zoom = std::clamp(std::exp(std::log(camera.zoom) + wheel * 0.2f), maxCameraZoom, minCameraZoom);
   }

   if (IsKeyReleased(KEY_ESCAPE)) {
      fadingOut = true;
   }
}

// Temporary way to switch, delete and place blocks. blockMap blocks must be in the same order as
// the blockIds map in objs/block.cpp.
static int index = 0;
static int size = 12;
static const char* blockMap[] {
   "grass", "dirt", "clay", "stone", "sand", "sandstone", "water", "bricks", "glass", "planks", "stone_bricks", "tiles"
};

void GameState::updatePhysics() {
   auto mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

   if (IsKeyPressed(KEY_E)) {
      index = (index + 1) % size;
   }

   if (IsKeyPressed(KEY_Q)) {
      index = (index == 0 ? size - 1 : index - 1);
   }

   if (map.isPositionValid(mousePos.x, mousePos.y)) {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
         map.deleteBlock(mousePos.x, mousePos.y);
      } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
         map.setBlock(mousePos.x, mousePos.y, blockMap[index]);
      } else if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) and map[mousePos.y][mousePos.x].type != Block::air) {
         index = map[mousePos.y][mousePos.x].id - 1;
      }
   }

   physicsTimer += GetFrameTime();
   if (physicsTimer >= .1f) {
      physicsTimer -= .1f;
   } else {
      return;
   }

   
   for (int y = mapSizeY - 1; y >= 0; --y) {
      for (int x = mapSizeX - 1; x >= 0; --x) {
         auto& block = map[y][x];

         if (block.type == Block::water) {
            if (map.is(x, y + 1, Block::air)) {
               map.moveBlock(x, y, x, y + 1);
            } else if (map.is(x - 1, y, Block::air) and map.is(x + 1, y, Block::air)) {
               if (chance(50)) {
                  goto moveWaterLeft;
               } else {
                  goto moveWaterRight;
               }
            } else if (map.is(x - 1, y, Block::air)) {
            moveWaterLeft:
               map.moveBlock(x, y, x - 1, y);
               --x; // Prevent the water tile from updating twice
            } else if (map.is(x + 1, y, Block::air)) {
            moveWaterRight:
               map.moveBlock(x, y, x + 1, y);
            }
         }

         if (block.type == Block::sand and (map.is(x, y + 1, Block::air) or map.is(x, y + 1, Block::water))) {
            map.moveBlock(x, y, x, y + 1);
         }

         if (block.type == Block::dirt and (map.is(x, y - 1, Block::air) or map.is(x, y - 1, Block::water)) and chance(1)) {
            map.setBlock(x, y, "grass");
         }

         if (block.type == Block::grass and not map.is(x, y - 1, Block::air) and not map.is(x, y - 1, Block::water) and chance(1)) {
            map.setBlock(x, y, "dirt");
         }
      }
   }
}

// Other functions

void GameState::render() {
   drawRect(BLUE);
   BeginMode2D(camera);

   auto crect = getCameraBounds(camera);
   auto maxY = std::min(mapSizeY, int((crect.y + crect.height)) + 1);
   auto maxX = std::min(mapSizeX, int((crect.x + crect.width)) + 1);

   for (int y = std::max(0, int(crect.y)); y < maxY; ++y) {
      for (int x = std::max(0, int(crect.x)); x < maxX; ++x) {
         auto& block = map[y][x];
         if (block.type == Block::air) {
            continue;
         }

         int ox = x;
         while (x < maxX and map[y][x].id == block.id) { ++x; }

         if (camera.zoom <= 12.5f) {
            DrawRectangle(ox, y, x - ox, 1, block.getColor());
         } else {
            drawTextureBlock(*block.tex, {(float)ox, (float)y, float(x - ox), 1.f});
         }
         --x;
      }
   }
   player.render();
   EndMode2D();

   drawTexture(ResourceManager::get().getTexture(blockMap[index]), {GetScreenWidth() - 75.f, GetScreenHeight() - 75.f}, {50.f, 50.f});
}

void GameState::change(States& states) {
   states.push_back(GameState::make());
}
