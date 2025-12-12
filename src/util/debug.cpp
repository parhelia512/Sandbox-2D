#include "util/debug.hpp"
#include <raylib.h>

// Debug globals

static bool debugMode = false;

// Debug functions

bool isDebugModeActive() {
   return debugMode;
}

// Update functions

void updateDebugMode() {
   if (IsKeyReleased(KEY_F3) && isProjectInDebug()) {
      debugMode = !debugMode;
   }
}
