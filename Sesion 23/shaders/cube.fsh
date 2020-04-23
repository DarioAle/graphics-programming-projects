#version 330

in vec3 colorToFS;
out vec4 pixelColor;

void main() {
	pixelColor = vec4(colorToFS, 1.0);
}