#include "game/loadingState.hpp"

// Includes

#include <fstream>
#include <nlohmann/json.hpp>
#include "game/menuState.hpp"
#include "mngr/resource.hpp"
#include "mngr/sound.hpp"
#include "util/format.hpp"
#include "util/position.hpp"
#include "util/text.hpp"
#include "util/random.hpp"
#include "util/render.hpp"

// Constants

namespace {
   constexpr float fadeTime = .5f;
   constexpr float waitTime = 1.f;
}

// Constructors

LoadingState::LoadingState() {
   splash = getSplashMessage();
   wrapText(splash, GetScreenWidth() - 50.f, 40, 1.f);
   ResourceManager::get().loadFont("andy"s, "assets/fonts/andy.ttf"s);
   ResourceManager::get().loadTexture("loading"s, "assets/sprites/loading.png"s);
}

// Update functions

void LoadingState::update() {
   rotation += GetFrameTime() * 360.f;
   switch (phase) {
   case Phase::fadingIn:  updateFadingIn();  break;
   case Phase::loading:   updateLoading();   break;
   case Phase::fadingOut: updateFadingOut(); break;
   }
}

void LoadingState::updateFadingIn() {
   fadeTimer += GetFrameTime();
   alpha = 1.f - fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      fadeTimer = alpha = 0.f;
      phase = Phase::loading;
   }
}

void LoadingState::updateLoading() {
   if (load == Load::fonts) {
      text = "Loading Fonts... "s;
      ResourceManager::get().loadFonts();
      load = Load::textures;
   } else if (load == Load::textures) {
      text = "Loading Textures... "s;
      ResourceManager::get().loadTextures();
      load = Load::sounds;
   } else if (load == Load::sounds) {
      text = "Loading Sounds... "s;
      SoundManager::get().loadSounds();
      load = Load::soundSetup;
   } else if (load == Load::soundSetup) {
      text = "Setting Up Sounds... "s;
      SoundManager::get().saveSound("click"s, {"click"s, "click2"s, "click3"s});
      SoundManager::get().saveSound("hover"s, {"hover"s, "hover2"s});
      load = Load::music;
   } else if (load == Load::music) {
      text = "Loading Music... "s;
      SoundManager::get().loadMusic();
      phase = Phase::fadingOut;
      SoundManager::get().play("load"s);
   }
}

void LoadingState::updateFadingOut() {
   waitTimer += GetFrameTime();
   if (waitTimer < waitTime) {
      return;
   }

   fadeTimer += GetFrameTime();
   alpha = fadeTimer / fadeTime;

   if (fadeTimer >= fadeTime) {
      alpha = 1.f;
      quitState = true;
   }
}

// Other functions

void LoadingState::render() {
   BeginDrawing();
      ClearBackground(BLACK);
      auto& tex = ResourceManager::get().getTexture("loading"s);
      std::string ltext = "Loading Done!"s;
      if (phase != Phase::fadingOut) {
         ltext = text + std::to_string((int)load) + "/"s + std::to_string((int)Load::count);
      }

      drawText(getScreenCenter(0.f, -175.f), ltext.c_str(), 80);
      drawText(getScreenCenter(0.f, 100.f), splash.c_str(), 40);
      drawTexture(tex, getScreenCenter(), {70.f, 70.f}, rotation);
      drawRect(Fade(BLACK, alpha));
   EndDrawing();
}

void LoadingState::change(States& states) {
   states.push_back(MenuState::make());
}

// Return the error message as the splash as the average user might not
// have the terminal opened
std::string LoadingState::getSplashMessage() {
   std::ifstream file ("assets/splash.json"s);
   if (not file.is_open()) {
      return "File 'assets/splash.json' does not exist."s;
   }

   nlohmann::json data;
   file >> data;
   file.close();

   if (not data.contains("splash"s) or not data["splash"s].is_array()) {
      return "Expected 'assets/splash.json' to contain array 'splash'."s;
   }
   
   auto splash = data["splash"s][random(0, data["splash"s].size() - 1)];
   if (not splash.is_string()) {
      return fmt::format("Expected 'assets/splash.json' element '{}' to be of type 'string', but it is '{}' instead.", splash.dump(), splash.type_name());
   }
   return splash;
}
