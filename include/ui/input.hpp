#ifndef UI_INPUT_HPP
#define UI_INPUT_HPP

#include <string>
#include <raylib.h>

// Input

struct Input {
   Rectangle rectangle;
   std::string text, defaultText;
   bool hovering = false, typing = false;
   int maxChars = 255;

   // Functions

   void update();
   void render();
};

#endif
