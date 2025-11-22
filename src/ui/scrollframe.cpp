#include "ui/scrollframe.hpp"
#include "mngr/resource.hpp"
#include "util/math.hpp"
#include "util/render.hpp"

// Update function

void Scrollframe::update() {
   float scrollFactor = GetMouseWheelMove();

   if (CheckCollisionPointRec(GetMousePosition(), rectangle) and not floatIsZero(scrollFactor)) {
      progress = clamp(progress + scrollFactor * 15.f * GetFrameTime() * (rectangle.height / scrollHeight), 0.f, 1.f);
   } else if (CheckCollisionPointRec(GetMousePosition(), {rectangle.x + rectangle.width - 50.f, scrollbarY, 50.f, scrollbarHeight})) {
      moving = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
   }
   
   if (not IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      moving = false;
   }
   
   if (moving) {
      float local = GetMouseY() - rectangle.y;
      progress = clamp(local / rectangle.height, 0.f, 1.f);
   }
}

// Render function

void Scrollframe::render() {
   scrollbarHeight = rectangle.height * (rectangle.height / scrollHeight);
   scrollbarY = rectangle.y + scrollbarHeight * progress;

   drawTextureNoOrigin(ResourceManager::get().getTexture("frame"), {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   drawTextureNoOrigin(ResourceManager::get().getTexture("scrollbar_background"), {rectangle.x + rectangle.width - 50.f, rectangle.y}, {50.f, rectangle.height});
   drawTextureNoOrigin(ResourceManager::get().getTexture("scrollbar_foreground"), {rectangle.x + rectangle.width - 50.f, scrollbarY}, {50.f, scrollbarHeight});
}
