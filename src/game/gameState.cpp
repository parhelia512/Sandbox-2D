#include "game/gameState.hpp"
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/input.hpp"
#include "mngr/sound.hpp"
#include "util/fileio.hpp"
#include "util/math.hpp"
#include "util/parallax.hpp"
#include "util/position.hpp"
#include "util/random.hpp"
#include <raymath.h>
#include <algorithm>
#include <cmath>
#include <unordered_map>

// Constants

constexpr float cameraFollowSpeed = 0.416f;
constexpr float minCameraZoom     = 12.5f;
constexpr float maxCameraZoom     = 200.0f;

constexpr int physicsTicks      = 8;
constexpr int lavaUpdateSpeed   = 3; // Lava updates 3x slower than water
constexpr int grassGrowSpeedMin = 100;
constexpr int grassGrowSpeedMax = 255;

// Constructors

GameState::GameState(const std::string &worldName)
: backgroundTexture(getRandomBackground()), foregroundTexture(getRandomForeground()), inventory(map, player, droppedItems), worldName(worldName) {
   const Vector2 center = getScreenCenter();
   
   // Init world and camera
   loadWorldData(worldName, player, camera.zoom, map, inventory, droppedItems);

   camera.zoom = clamp(camera.zoom, minCameraZoom, maxCameraZoom);
   camera.target = player.getCenter();
   camera.offset = center;
   camera.rotation = 0.0f;
   calculateCameraBounds();

   // Init UI
   continueButton.rectangle = {center.x, center.y, buttonWidth, buttonHeight};
   continueButton.text = "Continue";
   menuButton.rectangle = {continueButton.rectangle.x, continueButton.rectangle.y + buttonPaddingY, buttonWidth, buttonHeight};
   menuButton.text = "Save & Quit";
   pauseButton.rectangle = {GetScreenWidth() - buttonWidth / 2.f + 10.0f, GetScreenHeight() - buttonHeight / 2.f, buttonWidth, buttonHeight};
   pauseButton.text = "Pause";
   continueButton.texture = menuButton.texture = &getTexture("button");
}

GameState::~GameState() {
   inventory.discardItem();
   saveWorldData(worldName, player.position.x, player.position.y, camera.zoom, map, &inventory, &droppedItems);
   resetBackground();
}

// Update

void GameState::update() {
   updatePauseScreen();
   updateControls();
   updatePhysics();
}

void GameState::fixedUpdate() {
   camera.target = lerp(camera.target, player.getCenter(), cameraFollowSpeed);

   if (paused) {
      return;
   }
   player.updatePlayer(map);
   
   // Update physics
   physicsCounter = (physicsCounter + 1) % physicsTicks;
   if (physicsCounter != 0) {
      return;
   }

   lavaCounter = (lavaCounter + 1) % lavaUpdateSpeed;
   bool updateLava = (lavaCounter == 0);

   Rectangle physicsBounds = cameraBounds;
   Vector2 halfSize = {(cameraBounds.width - cameraBounds.x) / 2.0f, (cameraBounds.height - cameraBounds.y) / 2.0f};
   physicsBounds.x = max<int>(0, cameraBounds.x - halfSize.x);
   physicsBounds.y = max<int>(0, cameraBounds.y - halfSize.y);
   physicsBounds.width = min<int>(map.sizeX - 1, cameraBounds.width + halfSize.x);
   physicsBounds.height = min<int>(map.sizeY - 1, cameraBounds.height + halfSize.y);

   // Loop backwards to avoid updating most of the moving blocks twice
   for (int y = physicsBounds.height; y >= physicsBounds.y; --y) {
      for (int x = physicsBounds.width; x >= physicsBounds.x; --x) {
         BlockType type = map.blocks[y][x].type;

         if ((type & BlockType::lava) && updateLava) {
            updateLavaPhysics(x, y);
         } else if (type & BlockType::water) {
            updateWaterPhysics(x, y);
         } else if (type & BlockType::sand) {
            updateSandPhysics(x, y);
         } else if (type & BlockType::grass) {
            updateGrassPhysics(x, y);
         } else if (type & BlockType::dirt) {
            updateDirtPhysics(x, y);
         } else if (type & BlockType::torch) {
            updateTorchPhysics(x, y);
         }
      }
   }
}

// Update pause screen

void GameState::updatePauseScreen() {
   pauseButton.update(dt);
   if (pauseButton.clicked || handleKeyPressWithSound(KEY_ESCAPE)) {
      paused = !paused;
   }

   if (!paused) {
      return;
   }

   continueButton.update(dt);
   menuButton.update(dt);

   if (continueButton.clicked) {
      paused = false;
   }

   if (menuButton.clicked) {
      fadingOut = true;
   }
}

// Update controls

void GameState::updateControls() {
   if (!paused) {
      const float zoomFactor = isKeyPressed(KEY_EQUAL) - isKeyPressed(KEY_MINUS);
      if (zoomFactor != 0.f) {
         camera.zoom = std::clamp<float>(std::exp(std::log(camera.zoom) + zoomFactor * 0.2f), minCameraZoom, maxCameraZoom);
      }
      inventory.update();
   }
   calculateCameraBounds();
}

// Update physics

/************************************/
// Temporary way to switch, delete and place blocks. blockMap blocks must be in the same order as
// the blockIds map in objs/block.cpp. Everything between these multi-comments is temporary.
static int index = 0;
static int size = 26;
static const char *blockMap[] {
   "grass", "dirt", "clay", "stone", "sand", "sandstone", "water", "bricks", "glass", "planks", "stone_bricks", "tiles", "obsidian",
   "lava", "platform", "snow", "ice", "mud", "jungle_grass", "lamp", "torch",
   "sapling", "cactus_seed", "table", "chair", "door"
};
static bool drawWall = false;
static bool canDraw = false;
static Furniture obj;
inline FurnitureType getFurnitureType() {
   static std::unordered_map<int, FurnitureType> ftypes = {{
      {21, FurnitureType::sapling}, {22, FurnitureType::cactusSeed}, {23, FurnitureType::table}, {24, FurnitureType::chair},
      {25, FurnitureType::door},
   }};
   return ftypes.count(index) ? ftypes[index] : FurnitureType::none;
}
/************************************/

void GameState::updatePhysics() {
   if (paused) {
      return;
   }
   
   // Update furniture
   const Vector2 translatedMousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   for (Furniture &obj: map.furniture) {
      obj.update(map, player, translatedMousePos);
   }

   map.furniture.erase(std::remove_if(map.furniture.begin(), map.furniture.end(), [](Furniture &f) -> bool {
      return f.deleted;
   }), map.furniture.end());

   /************************************/
   // Move this to a different function later on!
   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

   if (isKeyPressed(KEY_Y)) {
      index = (index + 1) % size;
   }

   if (isKeyPressed(KEY_T)) {
      index = (index == 0 ? size - 1 : index - 1);
   }

   if (isKeyPressed(KEY_R)) {
      drawWall =! drawWall;
   }

   if (map.isPositionValid(mousePos.x, mousePos.y)) {
      FurnitureType ftype = getFurnitureType();
      canDraw = (drawWall || ftype != FurnitureType::none || !CheckCollisionRecs(player.getBounds(), {(float)(int)mousePos.x, (float)(int)mousePos.y, 1.f, 1.f}));

      if (isMouseDownOutsideUI(MOUSE_BUTTON_LEFT)) {
         map.deleteBlock(mousePos.x, mousePos.y, drawWall);
      } else if (isMouseDownOutsideUI(MOUSE_BUTTON_RIGHT) && canDraw && !map.isu(mousePos.x, mousePos.y, BlockType::furniture)) {
         if (ftype != FurnitureType::none) {
            generateFurniture(mousePos.x, mousePos.y, map, ftype, player.flipX);
         } else {
            map.setBlock(mousePos.x, mousePos.y, blockMap[index], drawWall);
         }
      } else if (isMousePressedOutsideUI(MOUSE_BUTTON_MIDDLE) && !((drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].type & BlockType::empty)) {
         index = (drawWall ? map.walls : map.blocks)[mousePos.y][mousePos.x].id - 1;
      }
   }
   /************************************/

   if (!droppedItems.empty()) {
      const Rectangle playerBounds = player.getBounds();
      
      for (auto &droppedItem: droppedItems) {
         droppedItem.update(cameraBounds, dt);

         if (!droppedItem.inBounds || !CheckCollisionRecs(playerBounds, droppedItem.getBounds())) {
            continue;
         }
         Item item {droppedItem.type, droppedItem.id, droppedItem.isFurniture, false, droppedItem.count};
         const int count = droppedItem.count;

         droppedItem.count = (inventory.placeItem(item) ? 0 : item.count);
         if (count != droppedItem.count) {
            playSound("pickup");
         }
      }

      droppedItems.erase(std::remove_if(droppedItems.begin(), droppedItems.end(), [](DroppedItem &i) -> bool {
         return i.flagForDeletion || i.count <= 0;
      }), droppedItems.end());
   }
}

// Block physic update functions

static constexpr unsigned char calculateFlowDown(unsigned char flow1, unsigned char flow2) {
   unsigned char availableSpace = maxWaterLayers - flow2;
   return min(availableSpace, flow1);
}

static void applyFlowDown(Block &block1, Block &block2) {
   unsigned char flowDown = calculateFlowDown(block1.value2, block2.value2);
   block1.value2 -= flowDown;
   block2.value2 += flowDown;
}

static void applyHalfFlowDown(Block &block1, Block &block2) {
   unsigned char flowDown = calculateFlowDown(block1.value2, block2.value2);
   unsigned char halfFlowDown = (flowDown == 1 ? 1 : flowDown / 2);
   
   block1.value2 -= halfFlowDown;
   block2.value2 += halfFlowDown;
}

void GameState::updateFluid(int x, int y) {
   // block.value2 is the fluid layers, which cannot exceed maxWaterLayers
   Block &block = map.blocks[y][x];
   BlockType liquidType = ((block.type & BlockType::water) ? BlockType::water : BlockType::lava);

   if (block.value2 == 0) {
      map.deleteBlock(x, y);
      return;
   }

   // Handle water going down
   if (map.is(x, y + 1, BlockType::torch) && block.value2 > minWaterLayers) {
      map.deleteBlock(x, y + 1);
   }

   if (map.is(x, y + 1, BlockType::empty)) {
      // Why? Because move function fails and I have no fucking idea why
      map.setBlock(x, y + 1, block.id);
      map.blocks[y + 1][x].value2 = block.value2;
      map.deleteBlock(x, y);
      return;
   } else if (map.is(x, y + 1, liquidType) && map.blocks[y + 1][x].value2 < maxWaterLayers) {
      applyFlowDown(block, map.blocks[y + 1][x]);
   }

   // Handle water going left. If first if is true, then it will automatically go over the second.
   if (map.is(x - 1, y, BlockType::torch) && block.value2 > minWaterLayers) {
      map.deleteBlock(x - 1, y);
   }

   if (map.is(x - 1, y, BlockType::empty)) {
      map.setBlock(x - 1, y, block.id);
      map.blocks[y][x - 1].value2 = 0;
   }

   if (map.is(x - 1, y, liquidType) && map.blocks[y][x - 1].value2 < block.value2 && map.blocks[y][x - 1].value2 <= maxWaterLayers) {
      applyHalfFlowDown(block, map.blocks[y][x - 1]);
   }

   // Handle water going right, same as with the left side
   if (map.is(x + 1, y, BlockType::torch) && block.value2 > minWaterLayers) {
      map.deleteBlock(x + 1, y);
   }
   
   if (map.is(x + 1, y, BlockType::empty)) {
      map.setBlock(x + 1, y, block.id);
      map.blocks[y][x + 1].value2 = 0;
   }

   if (map.is(x + 1, y, liquidType) && map.blocks[y][x + 1].value2 < block.value2 && map.blocks[y][x + 1].value2 <= maxWaterLayers) {
      applyHalfFlowDown(block, map.blocks[y][x + 1]);
   }
}

// Since lava updates 3x slower and in batch, make water turn
// nearby tiles into obsidian
void GameState::updateWaterPhysics(int x, int y) {
   Block &block = map.blocks[y][x];

   // What even is C++ syntax?
   for (const Vector2 &offset: {Vector2{1, 0}, Vector2{0, 1}, Vector2{-1, 0}, Vector2{0, -1}}) {
      if (!map.is(x + offset.x, y + offset.y, BlockType::lava)) {
         continue;
      }

      if (map.blocks[y + offset.y][x + offset.x].value2 < lavaLayerThreshold || (map.blocks[y + offset.y][x + offset.x].type & BlockType::furniture)) {
         map.deleteBlock(x + offset.x, y + offset.y);
         continue;
      }

      if (block.value2 >= lavaLayerThreshold) {
         map.setBlock(x + offset.x, y + offset.y, "obsidian");
      }
      map.deleteBlock(x, y);
   }

   if (block.type & BlockType::liquid) {
      updateFluid(x, y);
   }
}

void GameState::updateLavaPhysics(int x, int y) {
   updateFluid(x, y);
}

void GameState::updateSandPhysics(int x, int y) {
   if (map.isNotSolid(x, y + 1)) {
      map.moveBlock(x, y, x, y + 1);
      return;
   }

   bool leftEmpty  = map.isNotSolid(x - 1, y + 1);
   bool rightEmpty = map.isNotSolid(x + 1, y + 1);

   // Hacky solution, but works
   if (rightEmpty && leftEmpty && chance(50)) {
      rightEmpty = false;
   }

   if (rightEmpty) {
      map.moveBlock(x, y, x + 1, y + 1);
   } else if (leftEmpty) {
      map.moveBlock(x, y, x - 1, y + 1);
   }
}

void GameState::updateGrassPhysics(int x, int y) {
   if (!map.is(x, y - 1, BlockType::solid)) {
      return;
   }

   Block &block = map.blocks[y][x];
   if (block.value2 == 0) {
      block.value2 = random(grassGrowSpeedMin, grassGrowSpeedMax);
   }

   block.value += 1;
   if (block.value >= block.value2) {
      block.value = 0;
      block.value2 = 0;

      // This might be a tripping point in the future, when more dirt and
      // grass is added. I don't care though, I don't want to create a map
      // here, which'll also be a tripping point. Just define grass exactly
      // before dirt in objs/map.cpp, please.
      map.setBlock(x, y, block.id + 1);
   }
}

void GameState::updateDirtPhysics(int x, int y) {
   if (map.is(x, y - 1, BlockType::solid)) {
      return;
   }

   Block &block = map.blocks[y][x];
   if (block.value2 == 0) {
      block.value2 = random(grassGrowSpeedMin, grassGrowSpeedMax);
   }

   block.value += 1;
   if (block.value >= block.value2) {
      block.value = 0;
      block.value2 = 0;
   
      // Same as before. Just define grass exactly before dirt, so IDs
      // match right
      map.setBlock(x, y, block.id - 1);
   }
}

void GameState::updateTorchPhysics(int x, int y) {
   Block &block = map.blocks[y][x];
   block.value = (block.value + 1) % 5;

   bool downEmpty = !map.is(x, y + 1, BlockType::solid) && !map.is(x, y + 1, BlockType::furniture);

   if (downEmpty && map.isStable(x - 1, y)) {
      block.value2 = 2;
   } else if (downEmpty && map.isStable(x + 1, y)) {
      block.value2 = 3;
   } else if (downEmpty && !(map.walls[y][x].type & BlockType::empty)) {
      block.value2 = 4;
   } else if (!downEmpty && map.isStable(x, y - 1)) {
      block.value2 = 1;
   } else if (!downEmpty) {
      block.value2 = 0;
   } else {
      map.deleteBlock(x, y);
   }
}

// Render

void GameState::render() const {
   const float delta = (paused ? 0 : player.delta.x * dt);
   drawBackground(foregroundTexture, backgroundTexture, delta, delta, (paused ? 0.0f : 1.0f) * dt);

   BeginMode2D(camera);
   renderGame();
   EndMode2D();
   renderUI();
}

// Render game

void GameState::renderGame() const {
   map.render(droppedItems, player, accumulator, cameraBounds, camera);

   /************************************/
   // Scary method of rendering furniture and block preview correctly
   Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
   if (canDraw && map.isPositionValid(mousePos.x, mousePos.y)) {
      FurnitureType ftype = getFurnitureType();
      if (ftype != FurnitureType::none) {
         static BlockType oldBelow = BlockType::empty;
         static bool flippedX = false;
         
         BlockType below = (map.isPositionValid(mousePos.x, mousePos.y + obj.sizeY) ? map.blocks[mousePos.y + obj.sizeY][mousePos.x].type : BlockType::empty);
         
         if (ftype != obj.type || oldBelow != below || flippedX != player.flipX) {
            obj = getFurniture(mousePos.x, mousePos.y, map, ftype, player.flipX, true);
         }
         flippedX = player.flipX;
         oldBelow = below;
         obj.posX = mousePos.x;
         obj.posY = mousePos.y;
         obj.preview(map);
      } else {
         DrawTexturePro(getTexture(blockMap[index]), {0, 0, 8, 8}, {(float)(int)mousePos.x, (float)(int)mousePos.y, 1, 1}, {0, 0}, 0, Fade((drawWall ? wallTint : (map.isu(mousePos.x, mousePos.y, BlockType::furniture) ? RED : WHITE)), previewAlpha));
      }
   }
   /************************************/
}

// Render UI

void GameState::renderUI() const {
   inventory.render();

   if (paused) {
      continueButton.render();
      menuButton.render();
   }
   pauseButton.render();
}

// Change states

State* GameState::change() {
   return new MenuState();
}

// Helper functions

void GameState::calculateCameraBounds() {
   // formula I pulled out my ass that magically works
   camera.target.x = clamp(camera.target.x * camera.zoom, camera.offset.x, map.sizeX * camera.zoom - camera.offset.x) / camera.zoom;
   camera.target.y = clamp(camera.target.y * camera.zoom, camera.offset.y, map.sizeY * camera.zoom - camera.offset.y) / camera.zoom;

   cameraBounds = getCameraBounds(camera);
   cameraBounds.x = max(0, int(cameraBounds.x));
   cameraBounds.y = max(0, int(cameraBounds.y));
   cameraBounds.width = min(map.sizeX - 1, int(cameraBounds.x + cameraBounds.width) + 1);
   cameraBounds.height = min(map.sizeY - 1, int(cameraBounds.y + cameraBounds.height) + 1);
}
