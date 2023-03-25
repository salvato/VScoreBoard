#version 410

in vec3 vPosition;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void
main() {
    gl_Position = lightSpaceMatrix * model * vec4(vPosition, 1.0);
}
