#version 400

in float lightIntensity;

out vec4 FragColor;
uniform vec3 fragmentColor;

void
main() {
    FragColor = vec4(fragmentColor * lightIntensity, 1.0);
}
