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

         if (!mouseOnFrame(position, size)) {
            continue;
         }

         // Handle favoriting items
         if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_ALT) && open && item.id != 0) {
            playSound("click");
            item.favorite = !item.favorite;
            return;
         }

         // Handle trashing items
         if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_CONTROL) && open && item.id != 0) {
            if (item.favorite) {
               return;
            }

            playSound("trash");
            trashItem(item);
            item = Item{};
            return;
         }

         // When pressing on frames while the inventory is closed, select the item
         if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && y == 0) {
            if (!open) {
               playSound("click");
            }
            selectedX = x;
            selectedY = 0;
         }

         // Handle swapping/discarding items
         if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && open && anySelected) {
            playSound("click");

            if (&item == selectedItem.address || (selectedItem.fromTrash && item.favorite)) {
               discardItem();
               return;
            }

            if (selectedItem.fullSelect) {
               if (item.id == selectedItem.item.id) {
                  item.count += selectedItem.item.count;
                  item.favorite = (item.favorite || selectedItem.item.favorite);
                  *selectedItem.address = Item{};
               } else {
                  std::swap(item, *selectedItem.address);
               }
            } else {
               if (item.id == 0) {
                  item = selectedItem.item;
               } else if (item.id != selectedItem.item.id) {
                  discardItem();
                  return;
               } else {
                  item.count += selectedItem.item.count;
               }
            }
            anySelected = false;
            selectedItem.reset();
            return;
         }

         // Handle dragging items around
         if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && open && !anySelected && item.id != 0) {
            playSound("click");
            anySelected = true;
            selectedItem = {item, &item, true, false};
            return;
         }

         if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && open && item.id != 0) {
            if (!anySelected) {
               selectedItem = {item, &item, false, false};
               selectedItem.item.count = 1;
            } else if (item.id != selectedItem.item.id) {
               return;
            } else {
               selectedItem.item.count += 1;
            }

            item.count -= 1;
            if (item.count <= 0) {
               item = Item{};
            }

            playSound("click");
            anySelected = true;
            return;
         }
      }
   }

   // Handle trash frame
   if (open) {
      Vector2 trashPosition = getFramePosition(inventoryWidth - 1, inventoryHeight, false);
      Vector2 trashSize = getFrameSize(false);

      if (!mouseOnFrame(trashPosition, trashSize)) {
         handleDiscarding();
         return;
      }

      // Handle shift-clicking
      if (IsKeyDown(KEY_LEFT_SHIFT) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && trashedItem.id != 0) {
         if (placeItem(trashedItem)) {
            trashedItem = Item{};
         }
         playSound("click");
         return;
      }

      // Trash the item
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && anySelected) {
         if (selectedItem.item.favorite) {
            discardItem();
            return;
         }

         if (selectedItem.fullSelect) {
            *selectedItem.address = Item{};
         }

         playSound("trash");
         trashItem(selectedItem.item);
         selectedItem.reset();
         anySelected = false;
         return;
      }

      // Un-trash the item
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !anySelected && trashedItem.id != 0) {
         playSound("click");
         anySelected = true;
         selectedItem = {trashedItem, &trashedItem, true, true};
         return;
      }

      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && trashedItem.id != 0) {
         if (!anySelected) {
            selectedItem = {trashedItem, &trashedItem, false, false};
            selectedItem.item.count = 1;
         } else if (trashedItem.id != selectedItem.item.id) {
            return;
         } else {
            selectedItem.item.count += 1;
         }

         trashedItem.count -= 1;
         if (trashedItem.count <= 0) {
            trashedItem = Item{};
         }

         playSound("click");
         anySelected = true;
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
   if (selectedItem.address == &trashedItem) {
      if (selectedItem.fullSelect || trashedItem.id == 0) {
         trashedItem = selectedItem.item;
      } else {
         trashedItem.count += selectedItem.item.count;
      }
   } else if (!selectedItem.fullSelect) {
      // TODO: check for fails
      placeItem(selectedItem.item);
   }

   anySelected = false;
   selectedItem.reset();
}

void Inventory::trashItem(const Item &item) {
   if (item.id == trashedItem.id) {
      trashedItem.count += item.count;
   } else {
      trashedItem = item;
   }
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

bool Inventory::mouseOnFrame(const Vector2 &position, const Vector2 &size) {
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
   return getTexture(trashedItem.id != 0 ? "small_frame" : "small_frame_trash");
}

// Item functions

bool Inventory::placeItem(const Item &item) {
   Item *firstAvailableSpot = nullptr;
   
   // Check for equal items
   for (int y = 0; y < inventoryHeight; ++y) {
      for (int x = 0; x < inventoryWidth; ++x) {
         Item &it = items[y][x];

         if (it.id == item.id) {
            it.count += item.count;
            return true;
         } else if (!firstAvailableSpot && it.id == 0) {
            firstAvailableSpot = &it;
         }
      }
   }

   // Place at the first available spot
   if (firstAvailableSpot) {
      *firstAvailableSpot = item;
      return true;
   }
   return false;
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
         if (item.id != 0) {
            renderItem(item, position, false);
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
      if (trashedItem.id != 0) {
         renderItem(trashedItem, position, false);
      }
   }

   // Render selected item
   if (anySelected) {
      renderItem(selectedItem.item, GetMousePosition(), true);
   }
}

void Inventory::renderItem(Item &item, const Vector2 &position, bool isSelected) {
   Color drawColor = (isSelected ? Fade(WHITE, 0.75f) : WHITE);

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

   if (item.count != 1) {
      Vector2 textPosition = Vector2Subtract(Vector2Add(position, itemframeSize), itemframeIndexOffset);
      drawText(textPosition, std::to_string(item.count).c_str(), 25, drawColor);
   }
}
