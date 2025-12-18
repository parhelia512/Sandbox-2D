#ifndef OBJS_INVENTORY_HPP
#define OBJS_INVENTORY_HPP

#include "objs/item.hpp"
#include "util/config.hpp"

struct Inventory {
   Item items[inventoryHeight][inventoryWidth];
   int selectedX = 0, selectedY = 0;
   bool open = false;

   bool anySelected = false;
   SelectedItem selectedItem;
   Item trashedItem;

   // Update functions

   void update();

   // Helper functions

   void toggleInventoryOpen();
   void switchOnKeyPress(int key, int hotbarX);
   void switchOnMouseWheel();

   void handleDiscarding();
   void discardItem();

   // Frame functions

   Vector2 getFramePosition(float x, float y, bool isSelected);
   Vector2 getFrameSize(bool isSelected);
   bool mouseOnFrame(const Vector2 &position, const Vector2 &size);

   Texture& getFrameTexture(bool isSelected, bool isFavorite);
   Texture& getTrashTexture(bool trashOccupied);

   // Item functions
   
   bool placeItem(Item &item);
   int getItemStackSize(const Item &item);
   int addItemCount(Item &item1, Item &item2);

   // Render functions
   
   void render();
   void renderItem(Item &item, const Vector2 &position, bool isSelected);
};

#endif
