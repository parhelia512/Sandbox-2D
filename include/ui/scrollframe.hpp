#ifndef UI_SCROLLFRAME_HPP
#define UI_SCROLLFRAME_HPP

#include <raylib.h>

struct Scrollframe {
   Rectangle rectangle;
   bool moving = false;

   float scrollHeight = 0.f;
   float progress = 0.f;
   float scrollbarY = 0.f;

   float scrollbarHeight = 0.f;
   static inline const float scrollBarWidth = 56.667f;

   void update();
   void render() const;

   bool inFrame(const Rectangle &rect) const;
   float getOffsetY() const;
   float getScrollBarWidth() const;
};

#endif
