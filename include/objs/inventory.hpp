#ifndef OBJS_INVENTORY_HPP
#define OBJS_INVENTORY_HPP

#include "objs/item.hpp"
#include "util/config.hpp"

struct Inventory {
   Item items[inventoryHeight][inventoryWidth];
   int selectedX = 0, selectedY = 0;
   bool open = false;

   void update();
   void render();
};

#endif
