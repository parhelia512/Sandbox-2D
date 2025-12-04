#ifndef UTIL_PARALLAX_HPP
#define UTIL_PARALLAX_HPP

#include <raylib.h>

void drawParallaxTexture(const Texture &texture, float &progress, float speed);
Texture& getRandomBackground();
Texture& getRandomForeground();

#endif
