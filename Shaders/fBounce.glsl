#version 150
// Author: Adrian Purser
// License: MIT

#ifdef GL_ES
// Set default precision to medium
precision highp int;
precision highp float;
#endif


uniform vec4 shadow_colour = vec4(0.,0.,0.,.6);
uniform float shadow_height = 0.075;
uniform float bounces = 3.0;
const float PI = 3.14159265358;

uniform float progress;
uniform sampler2D texture0;
uniform sampler2D texture1;

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


vec4
transition (vec2 uv) {
  float time = progress;
  float stime = sin(time * PI / 2.);
  float phase = time * PI * bounces;
  float y = (abs(cos(phase))) * (1.0 - stime);
  float d = uv.y - y;
  return mix(
    mix(
      getToColor(uv),
      shadow_colour,
      step(d, shadow_height) * (1. - mix(
        ((d / shadow_height) * shadow_colour.a) + (1.0 - shadow_colour.a),
        1.0,
        smoothstep(0.95, 1., progress) // fade-out the shadow at the end
      ))
    ),
    getFromColor(vec2(uv.x, uv.y + (1.0 - y))),
    step(d, 0.0)
  );
}


void
main(void) {
    gl_FragColor = transition(v_texcoord);
}
