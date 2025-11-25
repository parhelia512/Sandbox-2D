#ifndef UTIL_BUTTON_HPP
#define UTIL_BUTTON_HPP

#include <string>
#include <raylib.h>

// Button

struct Button {
   Texture2D* texture = nullptr;
   Rectangle rectangle;
   std::string text;
   bool hovering = false, down = false, clicked = false;
   float scale = 1;

   // Functions

   void update(float offsetY = 0.f);
   void render(float offsetY = 0.f);

   Rectangle normalizeRect();
};

#endif
