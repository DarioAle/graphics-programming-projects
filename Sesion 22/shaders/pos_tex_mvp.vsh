#version 400

in vec3 position;
in vec2 texcoord;
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
out vec2 texcoordVF;

void main() {
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * 
                vec4(position, 1);
  texcoordVF = texcoord;
}
