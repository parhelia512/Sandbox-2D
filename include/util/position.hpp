#ifndef UTIL_POSITION_HPP
#define UTIL_POSITION_HPP

#include <raylib.h>

Vector2 getScreenSize();
Vector2 getScreenCenter(const Vector2 &offset = {0, 0});
Vector2 getOrigin(const Vector2 &size);
Vector2 getOrigin(const char *text, float fontSize, float spacing);

Rectangle getBox(const Texture &texture);
Rectangle getCameraBounds(const Camera2D &camera);

Vector2 lerp(const Vector2 &a, const Vector2 &b, float t);

#endif
