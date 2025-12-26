#include "game/state.hpp"
#include <raylib.h>

// Constants

constexpr float maxDT    = 0.25f;
constexpr float fadeTime = 0.4f;

// Update functions

void State::updateStateLogic() {
   if (fadingIn) {
      updateFadingIn();
      return;
   } else if (fadingOut) {
      updateFadingOut();
      return;
   }

   float frameTime = GetFrameTime();
   if (frameTime > maxDT) {
      frameTime = maxDT;
   }

   accumulator += frameTime;
   while (accumulator >= fixedUpdateDT) {
      fixedUpdate();
      accumulator -= fixedUpdateDT;
   }

   update(frameTime);
}

void State::updateFadingIn() {
   fadeTimer += GetFrameTime();
   alpha = 1.f - fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      fadeTimer = 0.f;
      alpha = 0.f;
      fadingIn = false;
   }
}

void State::updateFadingOut() {
   fadeTimer += GetFrameTime();
   alpha = fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      alpha = 1.f;
      fadingOut = false;
      quitState = true;
   }
}
