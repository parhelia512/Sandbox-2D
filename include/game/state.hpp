#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

// Polymorphic state

struct State {
   State() = default;
   virtual ~State() = default;

   // Virtual functions

   virtual void update() = 0;
   virtual void render() const = 0;
   virtual State* change() = 0;

   // Update functions

   void updateStateLogic();
   void updateFadingIn();
   void updateFadingOut();

   // Members

   bool quitState = false;
   bool fadingIn = true;
   bool fadingOut = false;

   float fadeTimer = 0.f;
   float alpha = 0.f;
};

#endif
