#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float progress;

out vec4 finalColor;

void main() {
   vec2 uv = fragTexCoord;
   if (uv.x > progress) {
      discard;
   }
   finalColor = texture(texture0, uv) * fragColor;
}
