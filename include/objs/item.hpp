#ifndef OBJS_ITEM_HPP
#define OBJS_ITEM_HPP

struct Item {
   enum Type { item, equipment, potion };

   Type type = Type::item;
   unsigned char id = 0;
   bool isFurniture = false;
   bool favorite = false;
   int count = 0;
};

struct SelectedItem {
   Item item;
   Item *address = nullptr;
   bool fullSelect = true;
   bool fromTrash = false;

   inline void reset() {
      item = Item{};
      address = nullptr;
      fullSelect = true;
      fromTrash = false;
   }
};

#endif
