#ifndef GAME_MENUSTATE_HPP
#define GAME_MENUSTATE_HPP

#include <vector>
#include "game/state.hpp"
#include "ui/button.hpp"
#include "ui/input.hpp"
#include "ui/scrollframe.hpp"

// Menu state

struct MenuState: public State {
   MenuState();
   ~MenuState() = default;

   // Update

   void update() override;
   void updateTitle();
   void updateLevelSelection();
   void updateLevelCreation();

   // Render

   void render() override;
   void renderTitle();
   void renderLevelSelection();
   void renderLevelCreation();

   // Other functions

   State* change() override;
   void loadWorlds();
   std::string getRandomWorldName();

private:
   enum class Phase { title, levelSelection, levelCreation };

   Button playButton, optionsButton, quitButton;
   Button backButton, newButton, createButton;
   std::vector<Button> worldButtons;
   Scrollframe worldFrame;
   Input worldName;

   std::string selectedWorld;
   Phase phase = Phase::title;
   bool playing = false;
};

#endif
