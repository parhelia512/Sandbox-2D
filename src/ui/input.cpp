#include "mngr/sound.hpp"
#include "ui/input.hpp"
#include "util/format.hpp"
#include "util/input.hpp"
#include "util/position.hpp"
#include "util/render.hpp"
#include <raymath.h>
#include <cmath>

// Constants

constexpr float inputTextWrapPadding = 10.f;
constexpr float inputTextFadeSpeed   = 0.3f;
constexpr int inputTextFadeMin       = 200;
constexpr int inputTextFadeValue     = 255 - inputTextFadeMin;

// Update

void Input::update() {
   bool wasTyping = typing;
   hovering = CheckCollisionPointRec(GetMousePosition(), rectangle);
   changed = false;   

   if (hovering) {
      setMouseOnUI(true);
   }

   if (hovering && !typing && isMousePressedUI(MOUSE_BUTTON_LEFT)) {
      typing = true;
   } else if (typing && (hovering ? isMousePressedUI(MOUSE_BUTTON_LEFT) : IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
      typing = false;
   }

   if ((IsKeyReleased(KEY_ENTER))) {
      typing = false;
   }

   if (typing) {
      std::size_t previous = text.size();
      if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE) || IsKeyPressedRepeat(KEY_DELETE)) && !text.empty()) {
         text.pop_back();

         while (IsKeyDown(KEY_LEFT_CONTROL) && !text.empty() && !std::isspace(text.back())) {
            text.pop_back();
         }
         changed = true;
      }

      char pressed = GetCharPressed();
      while (pressed != 0 && text.size() < (std::size_t)maxChars) {
         text += pressed;
         pressed = GetCharPressed();
         changed = true;
      }

      if (text.size() != previous) {
         playSound("hover");
      }
   }

   if (wasTyping != typing) {
      playSound("click");
   }

   if (!wasTyping && typing) {
      text.clear();
      changed = true;
   }
   ++counter;
}

// Render function

void Input::render() const {
   unsigned char value = (typing ? std::sin(counter * inputTextFadeSpeed) * inputTextFadeValue + inputTextFadeMin : 255) - (text.empty() ? inputTextFadeValue : 0);
   std::string wrapped = text;
   wrapText(wrapped, rectangle.width - inputTextWrapPadding, 35, 1);

   if (texture) {
      drawTextureNoOrigin(*texture, {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
   }
   drawText(Vector2Add({rectangle.x, rectangle.y}, getOrigin({rectangle.width, rectangle.height})), (text.empty() ? fallback : wrapped).c_str(), 35, Color{value, value, value, 255});
}
