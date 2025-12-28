#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform float time;
uniform float amplitude;
uniform float speed;
uniform int isTop;

out vec2 fragTexCoord;
out vec4 fragColor;
out float timeThing;

void main() {
   vec3 position = vertexPosition;
   if (isTop == 1 && vertexTexCoord.y < 1.0) {
      position.y += sin(time * speed + position.x) * amplitude;
   }

   gl_Position = mvp * vec4(position, 1.0);
   fragTexCoord = vertexTexCoord;
   fragColor = vertexColor;
   timeThing = time;
}
