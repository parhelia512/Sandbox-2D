#ifndef UTIL_BUTTON_HPP
#define UTIL_BUTTON_HPP

// Includes

#include <string>
#include <raylib.h>

// Button class

class Button {
public:
   Rectangle rectangle;
   std::string text;
   bool hovering = false, down = false, clicked = false;
   float scale = 1;

   void update();
   void render();
};

#endif
