
#version 400

in vec3 vertexPosition;
in vec3 vertexNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 worldVertexPosition;
out vec3 worldVertexNormal;


void main() {
  vec4 worldPosition = modelMatrix	 * vec4(vertexPosition,1);
  gl_Position = projMatrix * viewMatrix * worldPosition;
  worldVertexPosition = worldPosition.xyz;
  
  mat4 G = transpose(inverse(modelMatrix));
  worldVertexNormal = (G * vec4(vertexNormal, 0)).xyz;
  
  
}
