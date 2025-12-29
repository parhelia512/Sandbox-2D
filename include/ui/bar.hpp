#ifndef UI_BAR_HPP
#define UI_BAR_HPP

#include <raylib.h>

struct Bar {
   void update(float alpha);
   void render() const;

   // Members

   Texture2D *texture = nullptr;
   Color foregroundTint = WHITE;
   Color backgroundTint = WHITE;
   Rectangle rectangle;

   float progress = 1.0f;
   float progressInterpolation = 1.0f;
};

#endif
