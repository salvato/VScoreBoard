#version 410

out vec4 color;
void
main() { // Empty...
    gl_FragDepth = gl_FragCoord.z;
    color = vec4(1.0, 0.0, 0.0, 1.1);
}
