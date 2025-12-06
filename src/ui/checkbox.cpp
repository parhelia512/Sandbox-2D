#include "ui/checkbox.hpp"
#include "util/render.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"

void CheckBox::update() {
   if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), rectangle)) {
      checked = !checked;
      playSound("click");
   }
}

void CheckBox::render() {
   drawTextureNoOrigin((checked ? getTexture("checkbox_checked") : getTexture("checkbox_unchecked")), {rectangle.x, rectangle.y}, {rectangle.width, rectangle.height});
}
