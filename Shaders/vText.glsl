#version 400


in vec3 vPosition;
in vec3 vNormal;
out float lightIntensity;


uniform mat4 camera;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;


void
main() {
    gl_Position = view * camera * model * vec4(vPosition, 1.0);
    lightIntensity = abs((matrixNormal * vec4(vNormal, 1.0)).z);
}
