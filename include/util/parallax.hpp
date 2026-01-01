#ifndef UTIL_PARALLAX_HPP
#define UTIL_PARALLAX_HPP

#include <raylib.h>

void resetBackground();
void drawBackground(const Texture &fgTexture, const Texture &bgTexture, float bgSpeed, float fgSpeed, float daySpeed);

int getMoonPhase();
float getTimeOfDay();

void setMoonPhase(int moonPhase);
void setTimeOfDay(float timeOfDay);

Color getLightBasedOnTime();
Texture& getRandomBackground();
Texture& getRandomForeground();

#endif
