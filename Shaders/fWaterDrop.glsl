#version 150
// author: Paweł Płóciennik
// license: MIT
uniform float amplitude = 30;
uniform float speed = 30;

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



vec4
transition(vec2 p) {
    vec2 dir = p - vec2(.5);
    float dist = length(dir);

    if (dist > progress) {
        return mix(getFromColor( p), getToColor( p), progress);
    } else {
        vec2 offset = dir * sin(dist * amplitude - progress * speed);
        return mix(getFromColor( p + offset), getToColor( p), progress);
    }
}

void
main(void) {
    gl_FragColor = transition(v_texcoord);
}

