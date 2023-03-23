#version 410

//scene information
uniform mat4 model;
uniform mat4 camera;
uniform mat4 view;
uniform mat4 modelView;
uniform mat3 normalMatrix;
uniform vec4 lightPos;

//information from shape
in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexture;

//lighting + texture info
out vec3 N;
out vec3 L;
out vec3 E;
out vec2 texCoord;

void main() {

  N = normalMatrix * vNormal;
  L = (camera * lightPos).xyz - (modelView * vPosition).xyz;
  E = -(modelView * vPosition).xyz; // from pt to viewer

	texCoord = vTexture;
  gl_Position =  view*camera*model*vPosition;
}
