#version 330 core

#ifdef GL_ES
precision highp int;
precision highp float;
#endif

uniform mat4   mvp_matrix;
attribute vec4 a_position;
attribute vec2 a_texcoord;
varying vec2   v_texcoord;


void
main() {
    // Calculate vertex position in screen space
    gl_Position = mvp_matrix * a_position;
    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = a_texcoord;
}
