#include "objs/inventory.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "objs/map.hpp"
#include "util/render.hpp"
#include <raymath.h>

void Inventory::update() {
   if (IsKeyReleased(toggleInventoryKey)) {
      playSound("click");
      open = !open;
   }

   int lastSelectedX = selectedX;
   int lastSelectedY = selectedY;

   if (IsKeyReleased(KEY_ONE)) {
      selectedX = 0;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_TWO)) {
      selectedX = 1;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_THREE)) {
      selectedX = 2;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_FOUR)) {
      selectedX = 3;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_FIVE)) {
      selectedX = 4;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_SIX)) {
      selectedX = 5;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_SEVEN)) {
      selectedX = 6;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_EIGHT)) {
      selectedX = 7;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_NINE)) {
      selectedX = 8;
      selectedY = 0;
   }
   if (IsKeyReleased(KEY_ZERO)) {
      selectedX = 9;
      selectedY = 0;
   }

   float wheel = GetMouseWheelMove();
   if (wheel == 1.0f) {
      selectedX = (selectedX + 1) % 10;
   } else if (wheel == -1.0f) {
      selectedX = (selectedX == 0 ? 9 : selectedX - 1);
   }

   if (selectedX != lastSelectedX || selectedY != lastSelectedY) {
      playSound("hover");
   }
}

void Inventory::render() {
   for (int y = 0; y < (open ? inventoryHeight : 1); ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Vector2 position = Vector2Add(Vector2Multiply(itemframePadding, {(float)x, (float)y}), itemframeTopLeft);

         if (x == selectedX && y == selectedY) {
            drawTextureNoOrigin(getTexture("small_frame_selected"), Vector2Subtract(position, selectedItemFrameOffset), selectedItemFrameSize);
         } else {
            drawTextureNoOrigin(getTexture("small_frame"), position, itemframeSize);
         }

         // Render the item
         Item &item = items[y][x];
         bool isItemValid = (item.id != 0);

         if (isItemValid && !item.isFurniture) {
            drawTextureNoOrigin(getTexture(Block::getName(item.id)), Vector2Add(position, itemframeItemOffset), itemframeItemSize);
         } else if (isItemValid && item.isFurniture) {
            FurnitureTexture texture = Furniture::getFurnitureIcon(item.id);
            Vector2 newPos = Vector2Add(position, Vector2Scale(itemframeSize, 0.5f));
            Vector2 fSize = itemframeItemSize;

            if (texture.sizeX < texture.sizeY) {
               fSize.x *= texture.sizeX / texture.sizeY;
            } else if (texture.sizeX > texture.sizeY) {
               fSize.y *= texture.sizeY / texture.sizeX;
            }
            DrawTexturePro(texture.texture, {0, 0, (float)texture.sizeX, (float)texture.sizeY}, {newPos.x, newPos.y, fSize.x, fSize.y}, Vector2Scale(fSize, 0.5f), 0, WHITE);
         }

         if (isItemValid) {
            Vector2 textPosition = Vector2Subtract(Vector2Add(position, itemframeSize), itemframeIndexOffset);
            drawText(textPosition, std::to_string(item.count).c_str(), 25);
         }

         if (y == 0) {
            Vector2 textPosition = Vector2Add(position, itemframeIndexOffset);
            drawText(textPosition, std::to_string(x + 1).c_str(), 25);
         }
      }
   }
}
