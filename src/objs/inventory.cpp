#include "objs/inventory.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "objs/map.hpp"
#include "util/render.hpp"
#include <raymath.h>

void Inventory::update() {
   toggleInventoryOpen();

   // Handle switching
   int lastSelectedX = selectedX;
   int lastSelectedY = selectedY;

   switchOnKeyPress(KEY_ONE,   0);
   switchOnKeyPress(KEY_TWO,   1);
   switchOnKeyPress(KEY_THREE, 2);
   switchOnKeyPress(KEY_FOUR,  3);
   switchOnKeyPress(KEY_FIVE,  4);
   switchOnKeyPress(KEY_SIX,   5);
   switchOnKeyPress(KEY_SEVEN, 6);
   switchOnKeyPress(KEY_EIGHT, 7);
   switchOnKeyPress(KEY_NINE,  8);
   switchOnKeyPress(KEY_ZERO,  9);

   switchOnMouseWheel();

   if (selectedX != lastSelectedX || selectedY != lastSelectedY) {
      playSound("hover");
   }

   // Handle item operations
   int height = (open ? inventoryHeight : 1);
 
   for (int y = 0; y < height; ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         bool isSelected = (x == selectedX && y == selectedY);
         Item &item = items[y][x];
         
         Vector2 position = getFramePosition(x, y, isSelected);
         Vector2 size = getFrameSize(isSelected);

         if (!mouseClicked(position, size)) {
            continue;
         }

         // Handle favoriting items
         if (IsKeyDown(KEY_LEFT_ALT) && open && item.id != 0) {
            playSound("click");
            item.favorite = !item.favorite;
            return;
         }

         // Handle trashing items
         if (IsKeyDown(KEY_LEFT_CONTROL) && open && item.id != 0) {
            if (item.favorite) {
               return;
            }

            playSound("trash");
            wasTrashed = false;
            anyTrashed = true;
            trashedItem = std::move(item);
            item = Item{};
            return;
         }

         // When pressing on frames while the inventory is closed, select the item
         if (y == 0) {
            if (!open) {
               playSound("click");
            }
            selectedX = x;
            selectedY = 0;
         }

         // Handle swapping/discarding items
         if (open && anySelected) {
            playSound("click");

            if (&item == selectedItem) {
               discardItem();
               return;
            }

            if (wasTrashed) {
               anyTrashed = (item.id != 0);
            }
            std::swap(item, *selectedItem);
            wasTrashed = false;
            anySelected = false;
            selectedItem = nullptr;
            return;
         }

         // Handle dragging items around
         if (open && !anySelected && item.id != 0) {
            playSound("click");
            anySelected = true;
            selectedItem = &item;
            return;
         }
      }
   }

   // Handle trash frame
   if (open) {
      Vector2 trashPosition = getFramePosition(inventoryWidth - 1, inventoryHeight, false);
      Vector2 trashSize = getFrameSize(false);

      if (!mouseClicked(trashPosition, trashSize)) {
         handleDiscarding();
         return;
      }

      // Trash the item
      if (anySelected) {
         if (selectedItem->favorite) {
            discardItem();
            return;
         }

         playSound("click");
         anyTrashed = true;
         trashedItem = *selectedItem;

         if (!wasTrashed) {
            *selectedItem = Item{};
         }
         selectedItem = nullptr;
         anySelected = false;
         return;
      }

      // Un-trash the item
      if (!anySelected && anyTrashed) {
         playSound("click");
         anySelected = true;
         wasTrashed = true;
         selectedItem = &trashedItem;
         anyTrashed = false;
         return;
      }
   }
}

// Helper functions

void Inventory::toggleInventoryOpen() {
   if (IsKeyReleased(toggleInventoryKey)) {
      playSound("click");
      open = !open;
   }
}

void Inventory::switchOnKeyPress(int key, int hotbarX) {
   if (IsKeyPressed(key)) {
      selectedX = hotbarX;
      selectedY = 0;
   }
}

void Inventory::switchOnMouseWheel() {
   float wheel = GetMouseWheelMove();
   if (wheel >= 1.0f) {
      selectedX = (selectedX + 1) % 10;
   } else if (wheel <= -1.0f) {
      selectedX -= 1;
      if (selectedX < 0) selectedX = 9;
   }
}

void Inventory::handleDiscarding() {
   bool pressedOutside = (anySelected && open && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
   if (pressedOutside) {
      playSound("click");
   }

   if ((anySelected && !open) || pressedOutside) {
      discardItem();
   }
}

void Inventory::discardItem() {
   if (wasTrashed) {
      trashedItem = *selectedItem;
      anyTrashed = true;
   }

   wasTrashed = false;
   anySelected = false;
   selectedItem = nullptr;
}

// Frame functions

Vector2 Inventory::getFramePosition(float x, float y, bool isSelected) {
   Vector2 position = Vector2Add(Vector2Multiply(itemframePadding, {x, y}), itemframeTopLeft);

   if (isSelected) {
      return Vector2Subtract(position, selectedItemFrameOffset);
   }
   return position;
}

Vector2 Inventory::getFrameSize(bool isSelected) {
   return (isSelected ? selectedItemFrameSize : itemframeSize);
}

bool Inventory::mouseClicked(const Vector2 &position, const Vector2 &size) {
   if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      return false;
   }

   Rectangle bounds {position.x, position.y, size.x, size.y};
   return CheckCollisionPointRec(GetMousePosition(), bounds);
}

Texture& Inventory::getFrameTexture(bool isSelected, bool isFavorite) {
   if (isSelected && isFavorite) {
      return getTexture("small_frame_favorite_selected");
   } else if (isSelected) {
      return getTexture("small_frame_selected");
   } else if (isFavorite) {
      return getTexture("small_frame_favorite");
   } else {
      return getTexture("small_frame");
   }
}

Texture& Inventory::getTrashTexture() {
   return getTexture(anyTrashed ? "small_frame" : "small_frame_trash");
}

// Render functions

void Inventory::render() {
   for (int y = 0; y < (open ? inventoryHeight : 1); ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Item &item = items[y][x];
         bool isSelected = (x == selectedX && y == selectedY);
         bool isFavorite = item.favorite;

         Vector2 position = getFramePosition(x, y, isSelected);
         Vector2 size = getFrameSize(isSelected);

         drawTextureNoOrigin(getFrameTexture(isSelected, isFavorite), position, size);
         if (item.id != 0 && (!anySelected || &item != selectedItem)) {
            renderItem(item, position);
         }

         if (y == 0) {
            Vector2 textPosition = Vector2Add(position, itemframeIndexOffset);
            drawText(textPosition, std::to_string(x + 1).c_str(), 25);
         }
      }
   }

   // Render trash frame if inventory is open
   if (open) {
      Vector2 position = getFramePosition(inventoryWidth - 1, inventoryHeight, false);
      Vector2 size = getFrameSize(false);

      drawTextureNoOrigin(getTrashTexture(), position, size);
      if (anyTrashed) {
         renderItem(trashedItem, position);
      }
   }

   // Render selected item
   if (anySelected) {
      renderItem(*selectedItem, GetMousePosition());
   }
}

void Inventory::renderItem(Item &item, const Vector2 &position) {
   Color drawColor = (anySelected &&& item == selectedItem ? Fade(WHITE, 0.75f) : WHITE);

   if (!item.isFurniture) {
      drawTextureNoOrigin(getTexture(Block::getName(item.id)), Vector2Add(position, itemframeItemOffset), itemframeItemSize, drawColor);
   } else if (item.isFurniture) {
      FurnitureTexture texture = Furniture::getFurnitureIcon(item.id);
      Vector2 newPos = Vector2Add(position, Vector2Scale(itemframeSize, 0.5f));
      Vector2 fSize = itemframeItemSize;

      if (texture.sizeX < texture.sizeY) {
         fSize.x *= texture.sizeX / texture.sizeY;
      } else if (texture.sizeX > texture.sizeY) {
         fSize.y *= texture.sizeY / texture.sizeX;
      }
      DrawTexturePro(texture.texture, {0, 0, (float)texture.sizeX, (float)texture.sizeY}, {newPos.x, newPos.y, fSize.x, fSize.y}, Vector2Scale(fSize, 0.5f), 0, drawColor);
   }

   if (item.count > 1) {
      Vector2 textPosition = Vector2Subtract(Vector2Add(position, itemframeSize), itemframeIndexOffset);
      drawText(textPosition, std::to_string(item.count).c_str(), 25, drawColor);
   }
}
