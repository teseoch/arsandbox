#pragma once

// ----------------------------- GLSL Shaders --------------------------------
static const char *kVS = R"GLSL(
#version 330 core
uniform vec2 u_projQuad[4];
uniform vec2 u_screenSize;
out vec2 v_st;
void main(){
  int i = gl_VertexID;
  vec2 p = u_projQuad[i];
  vec2 ndc = vec2(
    (p.x / u_screenSize.x) * 2.0 - 1.0,
    1.0 - (p.y / u_screenSize.y) * 2.0
  );
  gl_Position = vec4(ndc, 0.0, 1.0);

  if(i==0) v_st = vec2(0.0,0.0);
  if(i==1) v_st = vec2(1.0,0.0);
  if(i==2) v_st = vec2(1.0,1.0);
  if(i==3) v_st = vec2(0.0,1.0);
}
)GLSL";

static const char *kFS = R"GLSL(
#version 330 core
in vec2 v_st;
out vec4 FragColor;

uniform sampler2D u_depthTex;   // GL_R32F, mm
uniform sampler1D  u_colormapTex;

uniform vec2  u_depthUVQuad[4];  // normalized [0,1]

uniform float u_depthMinMm;
uniform float u_depthMaxMm;
uniform float u_gamma;


// bilinear warp from quad coords to depth UV
vec2 warpUV(vec2 st){
  vec2 a = mix(u_depthUVQuad[0], u_depthUVQuad[1], st.x);
  vec2 b = mix(u_depthUVQuad[3], u_depthUVQuad[2], st.x);
  return mix(a, b, st.y);
}

// isolines
vec3 addIsolines(vec3 baseColor, float h, float step){
  if(step <= 0.0) return baseColor;
  float x = h / step;
  float f = abs(fract(x) - 0.5);
  float line = smoothstep(0.48, 0.50, f);
  line = 1.0 - line;
  return mix(baseColor, vec3(1.0), 0.65 * line);
}

void main(){
  vec2 uv = warpUV(v_st);
  float d = texture(u_depthTex, uv).r;
  if(d < 1.0){
    FragColor = vec4(0,0,0,1);
    return;
  }

  float dn = clamp((d - u_depthMinMm) / (u_depthMaxMm - u_depthMinMm), 0.0, 1.0);
  float h  = (1.0 - dn);

  float t = clamp(h, 0.0, 1.0);
  t = pow(t, u_gamma);
  vec3 col = texture(u_colormapTex, t).rgb;

  FragColor = vec4(col, 1.0);
}
)GLSL";
