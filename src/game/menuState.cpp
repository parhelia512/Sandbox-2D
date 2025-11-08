#include "game/menuState.hpp"

// Includes

#include <raylib.h>

// Constructors

MenuState::MenuState() {

}

// Update

void MenuState::update() {

}

// Other functions

void MenuState::render() {
   BeginDrawing();
      ClearBackground(BLACK);
   EndDrawing();
}

void MenuState::change(States& states) {
   
}
