#include "mngr/resource.hpp"
#include "ui/input.hpp"
#include "mngr/sound.hpp"
#include "util/render.hpp"

// Update function

void Input::update() {
   bool wasTyping = typing;
   hovering = CheckCollisionPointRec(GetMousePosition(), rectangle);

   if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      typing = (hovering and not typing);
   }

   if ((IsKeyReleased(KEY_ENTER))) {
      typing = false;
   }

   if (typing) {
      auto previous = text.size();
      if ((IsKeyPressed(KEY_BACKSPACE) or IsKeyPressedRepeat(KEY_BACKSPACE) or IsKeyPressed(KEY_DELETE) or IsKeyPressedRepeat(KEY_DELETE)) and not text.empty()) {
         text.pop_back();
      }

      char pressed = GetCharPressed();
      while (pressed != 0 and text.size() < maxChars) {
         text += pressed;
         pressed = GetCharPressed();
      }

      if (text.size() != previous) {
         SoundManager::get().play("hover");
      }
   }

   if (wasTyping != typing) {
      SoundManager::get().play("click");
   }
}

// Render function

void Input::render() {
   drawTextureNoOrigin(ResourceManager::get().getTexture("button"), {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   drawText({rectangle.x + rectangle.width / 2.f, rectangle.y + rectangle.height / 2.f}, (text.empty() ? defaultText : text).c_str(), 35);
}
