#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
uniform mat4 modelMatrix;
uniform vec3 materialColor;

out vec3 vertexPositionToGS;
out vec3 vertexColorToGS;
out vec3 vertexNormalToGS;

void main() {

	vec4 worldPosition4 = modelMatrix * vec4(vertexPosition, 1);	
	vec3 worldPosition3 = worldPosition4.xyz;
	vec4 modelNormal = transpose(inverse(modelMatrix)) * vec4(vertexNormal, 0);
	vec3 unitNormal  = normalize(modelNormal.xyz);
	
	vec3 lightPosition = vec3(0, 0, 1);
	vec3 lightDirection = -normalize(worldPosition3 - lightPosition);
	
	float factorD = clamp(dot(unitNormal, lightDirection), 0, 1);
	vec3 ambientColor = materialColor / 3;
	 

	vertexColorToGS    = clamp(ambientColor + materialColor * factorD, 0, 1);;
	vertexPositionToGS = vertexPosition;
	vertexNormalToGS   = vertexNormal;
}

// void main1_without_gsh() {
// 	vec4 worldPosition4 = modelMatrix * vec4(vertexPosition, 1);	
// 	gl_Position = projectionMatrix * viewMatrix * worldPosition4;
	
// 	vec3 worldPosition3 = worldPosition4.xyz;
// 	vec4 modelNormal = transpose(inverse(modelMatrix)) * vec4(vertexNormal, 0);
// 	vec3 unitNormal  = normalize(modelNormal.xyz);
// 	vec3 lightPosition = vec3(0, 0, 1);
// 	vec3 lightDirection = -normalize(worldPosition3 - lightPosition);
// 	float factorD = clamp(dot(unitNormal, lightDirection), 0, 1);
// 	vec3 ambientColor = materialColor / 3;
// 	colorToFS = clamp(ambientColor + materialColor * factorD, 0, 1);
// }