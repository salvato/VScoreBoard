#ifdef GL_ES
precision highp float;
#endif

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform int anim;
uniform float alpha;
varying vec2 v_texcoord;

void
main() {
    if(anim == 0) {
        vec4 texColor0 = texture2D(texture0, v_texcoord);
        gl_FragColor = texColor0;
    }
    else if(anim == 1) {
        vec4 texColor0 = texture2D(texture0, v_texcoord);
        vec4 texColor1 = texture2D(texture1, v_texcoord);
        gl_FragColor = texColor0*alpha + texColor1*(1.0-alpha);
    }
    else {
        gl_FragColor = texture2D(texture0, v_texcoord);
    }
}
