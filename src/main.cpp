#include "game/loadingState.hpp"
#include "mngr/sound.hpp"
#include "ui/popup.hpp"
#include "util/input.hpp"
#include "util/render.hpp"
#include <raylib.h>
#include <cstdlib>
#include <ctime>

int main() {
   // Initialize the game
   
   srand(time(nullptr));
   SetConfigFlags(FLAG_VSYNC_HINT);
   InitWindow(GetMonitorWidth(GetCurrentMonitor()), GetMonitorHeight(GetCurrentMonitor()), "Sandbox-2D");
   ToggleFullscreen();
   SetTargetFPS(60);
   
   InitAudioDevice();
   SetExitKey(KEY_NULL);
   State *current = new LoadingState();
   
   // Run the game

   while (!WindowShouldClose()) {
      if (current->quitState) {
         State *newState = current->change();
         delete current;
         current = newState;
      }

      if (!current) {
         break;
      }

      resetInput();
      updateMusic();

      updatePopups();
      if (!anyPopups()) {
         current->updateStateLogic();
      }

      BeginDrawing();
         ClearBackground(BLACK);
         current->render();

         renderPopups();
         drawRect(Fade(BLACK, current->alpha));
      EndDrawing();
   }

   // De-initialize the game

   if (current) {
      delete current;
   }
   CloseWindow();
   CloseAudioDevice();
}
