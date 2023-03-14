#ifdef GL_ES
// Set default precision to medium
precision highp int;
precision highp float;
#endif

uniform sampler2D texture0;

varying vec2 v_texcoord;


void
main() {
    // Set fragment color from texture
    gl_FragColor = texture2D(texture0, v_texcoord);
}


