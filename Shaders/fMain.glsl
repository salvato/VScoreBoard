#version 330 core

void
main(void) {
    gl_FragColor = transition(v_texcoord);
}
