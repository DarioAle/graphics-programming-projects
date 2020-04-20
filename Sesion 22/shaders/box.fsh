#version 400

in vec2 texcoordVF;
out vec4 pixelColor; 

void main() {
   vec2 point = vec2(0.5, 0.5);
   // float d = length(texcoordVF - point);
   // float MAX  = 0.7071;
   float d = abs(texcoordVF.x - point.x) + abs(texcoordVF.y - point.y);
   float MAX = 1.0;
   vec4 color;
   if(d < MAX / 3) color = vec4(d, 1 - d, 0,  1);
   else if(d < 2 * MAX / 3) color = vec4(0, d, 1 - d,  1);
   else color = vec4(1 - d, 0, d,  1);

   float d2 = length(texcoordVF - point);
   float MAX2 = 0.7071;
   if(d < MAX / 3) pixelColor = (vec4(d2, 1 - d2, 0,  1) + color) / 2;
   else if(d < 2 * MAX / 3) pixelColor = (vec4(0, d2, 1 - d2,  1) + color) / 2;
   else pixelColor = (vec4(1 - d2, 0, d2,  1) + color) / 2;
}