#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform float time;
uniform float amplitude;
uniform float speed;
uniform int isBottom;
uniform int isTop;

out vec2 fragTexCoord;
out vec4 fragColor;
out float timeThing;

void main() {
   vec3 position = vertexPosition;

   // A monster, I know
   if (!(isTop == 1 && isBottom == 1) && ((isTop == 1 && isBottom == 0 && vertexTexCoord.y == 1.0) || (isTop == 0 && isBottom == 1 && vertexTexCoord.y == 0.0) || (isTop == 0 && isBottom == 0))) {
      position.y += sin(time * speed + position.x) * amplitude;
   }

   gl_Position = mvp * vec4(position, 1.0);
   fragTexCoord = vertexTexCoord;
   fragColor = vertexColor;
   timeThing = time;
}
