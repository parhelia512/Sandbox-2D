#ifndef UI_CHECKBOX_HPP
#define UI_CHECKBOX_HPP

#include <raylib.h>
#include <string>

struct CheckBox {
   void update();
   void render() const;

   Rectangle rectangle;
   std::string keybind;
   bool checked = false;
};

#endif
