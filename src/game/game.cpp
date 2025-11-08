#include "game/game.hpp"

// Includes

#include <cstdlib>
#include <raylib.h>
#include "constants.hpp"
#include "game/menuState.hpp"
#include "mngr/sound.hpp"

// Constructors

Game::Game() {
   srand(time(nullptr));
   InitWindow(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()), title);
   ToggleFullscreen();
   SetTargetFPS(targetFPS);
   
   InitAudioDevice();
   SetExitKey(KEY_NULL);
   states.push_back(MenuState::make());
}

Game::~Game() {
   CloseWindow();
   CloseAudioDevice();
}

// Run function

void Game::run() {
   while (not WindowShouldClose()) {
      if (states.front()->quitState) {
         states.front()->change(states);
         states.pop_front();
      }

      if (states.empty()) {
         return;
      }

      SoundManager::get().update();
      states.front()->update();
      states.front()->render();
   }
}
