#version 330 core

// Author: Travis Fischer
// License: MIT
//
// Adapted from a Codrops article by Robin Delaporte
// https://tympanus.net/Development/DistortionHoverEffect

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


uniform sampler2D displacementMap;

uniform float strength = 0.5;

vec4 transition (vec2 uv) {
  float displacement = texture2D(displacementMap, uv).r * strength;

  vec2 uvFrom = vec2(uv.x + progress * displacement, uv.y);
  vec2 uvTo = vec2(uv.x - (1.0 - progress) * displacement, uv.y);

  return mix(
    getFromColor(uvFrom),
    getToColor(uvTo),
    progress
  );
}

void
main(void) {
    gl_FragColor = transition(v_texcoord);
}
