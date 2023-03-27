#version 410

out vec4 color;


void
main() { // Empty...
    gl_FragDepth = gl_FragCoord.z;
    color = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}
