#version 400

in vec2 texcoordVF;
uniform sampler2D myTexture;
out vec4 pixelColor; 

void main() {
   pixelColor = texture(myTexture, texcoordVF);
}

void main1() {
   pixelColor = texture(myTexture, texcoordVF);
   if(pixelColor.r >= 0.9 && pixelColor.g >= 0.9 && pixelColor.b >= 0.9) discard;
}
