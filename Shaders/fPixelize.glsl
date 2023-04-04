#version 330 core

// Author: gre
// License: MIT
// forked from https://gist.github.com/benraziel/c528607361d90a072e98

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float progress;
varying vec2 v_texcoord;


vec4
getFromColor(vec2 p) {
    vec4 tempc;
    tempc = texture2D(texture0, vec2(p.x, p.y));
    return tempc;
}

vec4
getToColor(vec2 p) {
    vec4 tempc;
    tempc = texture2D(texture1, vec2(p.x, p.y));
    return tempc;
}


// minimum number of squares (when the effect is at its higher level)
uniform ivec2 squaresMin = ivec2(20);
// zero disable the stepping
uniform int steps = 50;

float d = min(progress, 1.0 - progress);
float dist = steps>0 ? ceil(d * float(steps)) / float(steps) : d;
vec2 squareSize = 2.0 * dist / vec2(squaresMin);

vec4 transition(vec2 uv) {
  vec2 p = dist>0.0 ? (floor(uv / squareSize) + 0.5) * squareSize : uv;
  return mix(getFromColor(p), getToColor(p), progress);
}

void
main(void) {
    gl_FragColor = transition(v_texcoord);
}
