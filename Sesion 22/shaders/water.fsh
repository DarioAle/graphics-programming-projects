#version 400

in vec2 texcoordVF1;
in vec2 texcoordVF2;
uniform sampler2D texture1;
uniform sampler2D texture2;
out vec4 pixelColor; 

void main() {
   pixelColor = // texture(texture1, texcoordVF1) *
                texture(texture2, texcoordVF2) * 
                vec4(1, 1, 1, 0.5);
}
