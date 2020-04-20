#version 400

in vec3 position;
in vec2 texcoord;
uniform float time;
uniform vec2 windowSize;
out vec2 texcoordVF1;
out vec2 texcoordVF2;

void main() {
  gl_Position = vec4(position, 1);
  texcoordVF1 = texcoord;
  texcoordVF2 = texcoord;
}

float fx(float t) {
	return 0.2 * t; // cos(t * 0.3);
}

float fy(float t) {
	return 0.3 * sin(t * 0.4);
}

void main1() {
  gl_Position = vec4(position, 1);
  texcoordVF1 = vec2(texcoord.x + fx(time), texcoord.y + fy(time));
  if(texcoord.x > 0) texcoordVF1.x = windowSize.x / 400.0 + fx(time);
  if(texcoord.y > 0) texcoordVF1.y = windowSize.y / 400.0 + fy(time);
	
  texcoordVF2 = vec2(texcoord.x + fy(time), texcoord.y + fx(time));
  if(texcoord.x > 0) texcoordVF2.x = windowSize.x / 400.0 + fy(time);
  if(texcoord.y > 0) texcoordVF2.y = windowSize.y / 400.0 + fx(time);
}
