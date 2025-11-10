#include "util/button.hpp"

// Includes

#include "mngr/resource.hpp"
#include "mngr/sound.hpp"

using namespace std::string_literals;

// Update function

void Button::update() {
   bool was_hovering = hovering;
   hovering = CheckCollisionPointRec({GetMouseX() + rectangle.width / 2.f, GetMouseY() + rectangle.height / 2.f}, rectangle);
   down = hovering and IsMouseButtonDown(MOUSE_LEFT_BUTTON);
   clicked = hovering and IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

   if (down) {
      scale = std::max(scale * .975f, .9f);
   } else if (hovering) {
      scale = std::min(scale * 1.025f, 1.1f);
   } else if (scale != 1.f) {
      scale = (scale < 1.f ? std::min(1.f, scale * 1.025f) : std::max(1.f, scale * .975f));
   }

   if (not was_hovering and hovering) {
      SoundManager::get().play("hover"s);
   }

   if (clicked) {
      SoundManager::get().play("click"s);
   }
}

// Render function

void Button::render() {
   auto& fon = ResourceManager::get().getFont("andy"s);
   auto& tex = ResourceManager::get().getTexture("button"s);
   float nw = rectangle.width * scale, nh = rectangle.height * scale;
   Vector2 tsize = MeasureTextEx(fon, text.c_str(), 35 * scale, 1.f);

   DrawTexturePro(tex, {0.f, 0.f, (float)tex.width, (float)tex.height}, {rectangle.x, rectangle.y, nw, nh}, {nw / 2.f, nh / 2.f}, 0.f, WHITE);
   DrawTextPro(fon, text.c_str(), {rectangle.x, rectangle.y}, {tsize.x / 2.f, tsize.y / 2.f}, 0.f, 35 * scale, 1.f, WHITE);
}
