# version 400

in vec3 vertexColorToGS[];
in vec3 vertexPositionToGS[];
in vec3 vertexNormalToGS[];

uniform mat4 projectionMatrix, viewMatrix, modelMatrix;
uniform bool shot;

out vec3 colorToFS;

layout (triangles, invocations = 1) in;
layout (triangle_strip, max_vertices = 6) out;

void main() {
    for(int i = 0; i < vertexPositionToGS.length(); i ++) {
        vec3 vp = vertexPositionToGS[i];
        colorToFS = vertexColorToGS[i];
        if(shot) {
            vp = vp * 0.8 + vertexNormalToGS[i] * 0.2;
            // colorToFS /= 2;
        }
        gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vp, 1);
        EmitVertex();
    }
    EndPrimitive();

    if(shot) {
        for(int i = 0; i < vertexPositionToGS.length(); ++i) {
            vec3 vp = vertexPositionToGS[i];
            vp = vp * 0.4 + 0.6 * vertexNormalToGS[i];
            // colorToFS  = 0.65 *  vertexColorToGS[i];
            colorToFS  = vertexColorToGS[i];
            gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vp, 1);
            EmitVertex();
        }
    }
    EndPrimitive();
}
