#version 400

in vec3 vertexPosition;
in vec3 vertexColor;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec3 worldVertexPosition;
out vec3 worldVertexNormal;
out vec3 vertexColorToFS;

void main() {
  vec4 worldPosition = modelMatrix * vec4(vertexPosition, 1);
  gl_Position = projectionMatrix * viewMatrix * worldPosition;
  worldVertexPosition = worldVertexNormal.xyz;

  mat4 G = transpose(inverse(modelMatrix));
  worldVertexNormal = (G * vec4(vertexNormal, 0)).xyz;

  vertexColorToFS = vertexColor;
}
