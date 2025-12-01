#include "mngr/resource.hpp"
#include "util/position.hpp"
#include <raymath.h>

Vector2 getScreenSize() {
   return {(float)GetScreenWidth(), (float)GetScreenHeight()};
}

Vector2 getScreenCenter(const Vector2 &offset) {
   return Vector2Add(getOrigin(getScreenSize()), offset);
}

Vector2 getOrigin(const Vector2 &size) {
   return Vector2Scale(size, 0.5f);
}

Vector2 getOrigin(const char *text, float fontSize, float spacing) {
   return getOrigin(MeasureTextEx(getFont("andy"), text, fontSize, spacing));
}

Rectangle getBox(const Texture &texture) {
   return {0, 0, (float)texture.width, (float)texture.height};
}

Rectangle getCameraBounds(const Camera2D &camera) {
   Vector2 pos = GetScreenToWorld2D({0, 0}, camera);
   Vector2 size = Vector2Scale(getScreenSize(), 1.f / camera.zoom);
   return {pos.x, pos.y, size.x, size.y};
}

Vector2 lerp(const Vector2 &a, const Vector2 &b, float t) {
   return Vector2Add(a, Vector2Scale(Vector2Subtract(b, a), Clamp(t, 0, 1)));
}
