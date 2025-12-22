#ifndef GAME_GAMESTATE_HPP
#define GAME_GAMESTATE_HPP

#include "game/state.hpp"
#include "objs/inventory.hpp"
#include "objs/player.hpp"
#include "ui/button.hpp"

// Game state

struct GameState: public State {
   // Constructors

   GameState(const std::string &worldName);
   ~GameState();

   // Update

   void update() override;
   void updatePauseScreen();
   void updateControls();
   void updatePhysics();

   // Render

   void render() const override;
   void renderGame() const;
   void renderUI() const;

   // Change states

   State* change() override;

   // Helpler functions

   void calculateCameraBounds();

   // Members

   const Texture &backgroundTexture, &foregroundTexture;

   Map map;
   Player player;

   Camera2D camera;
   Rectangle cameraBounds;

   Inventory inventory;
   Button continueButton, menuButton, pauseButton;

   std::vector<DroppedItem> droppedItems;
   std::string worldName;

   float physicsTimer = 0.0f;
   float updateTimer = 0.0f;
   bool paused = false;
};

#endif
