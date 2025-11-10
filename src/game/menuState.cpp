#include "game/menuState.hpp"

// Includes

#include <raylib.h>
#include "util/position.hpp"
#include "util/render.hpp"

using namespace std::string_literals;

// Constants

namespace {
   constexpr float fadeTime = .25f;
}

// Constructors

MenuState::MenuState() {
   playButton.rectangle = {GetScreenWidth() / 2.f, GetScreenHeight() / 2.f, 210.f, 70.f};
   playButton.text = "Play"s;
   optionsButton.rectangle = {playButton.rectangle.x, playButton.rectangle.y + 90.f, 210.f, 70.f};
   optionsButton.text = "Options"s;
   quitButton.rectangle = {optionsButton.rectangle.x, optionsButton.rectangle.y + 90.f, 210.f, 70.f};
   quitButton.text = "Quit"s;
}

// Update

void MenuState::update() {
   switch (phase) {
   case Phase::fadingIn:  updateFadingIn();  break;
   case Phase::updating:  updateUpdating();  break;
   case Phase::fadingOut: updateFadingOut(); break;
   }
}

void MenuState::updateFadingIn() {
   fadeTimer += GetFrameTime();
   alpha = 1.f - fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      fadeTimer = alpha = 0.f;
      phase = Phase::updating;
   }
}

void MenuState::updateUpdating() {
   playButton.update();
   optionsButton.update();
   quitButton.update();

   if (playButton.clicked) {
      phase = Phase::fadingOut;
   }

   if (optionsButton.clicked) {

   }

   if (quitButton.clicked) {
      phase = Phase::fadingOut;
   }
}

void MenuState::updateFadingOut() {
   fadeTimer += GetFrameTime();
   alpha = fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      alpha = 1.f;
      quitState = true;
   }
}

// Other functions

void MenuState::render() {
   BeginDrawing();
      ClearBackground(BLACK);
      drawText(getScreenCenter(0.f, -200.f), "TERRARIA", 180);
      playButton.render();
      optionsButton.render();
      quitButton.render();
      drawRect(Fade(BLACK, alpha));
   EndDrawing();
}

void MenuState::change(States& states) {
   
}
