#ifndef UI_BUTTON_HPP
#define UI_BUTTON_HPP

#include <raylib.h>
#include <string>

// Constants

constexpr inline float buttonWidth    = 210.0f;
constexpr inline float buttonHeight   = 70.0f;
constexpr inline float buttonPaddingX = buttonWidth + 20.0f;
constexpr inline float buttonPaddingY = buttonHeight + 20.0f;

// Button

struct Button {
   Texture2D *texture = nullptr;
   Rectangle rectangle;
   std::string text;
   bool hovering = false, down = false, clicked = false;
   bool favorite = false, disabled = false;
   float scale = 1;

   void update(float offsetY = 0.f);
   void render(float offsetY = 0.f) const;
   Rectangle normalizeRect() const;
};

#endif
