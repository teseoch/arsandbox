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
uniform sampler2D u_flowTex;
uniform sampler2D u_flowTex2;

uniform vec3 u_flowColor1;
uniform vec3 u_flowColor2;

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

  float flow1 = texture(u_flowTex, v_st).r;
  flow1 = clamp(flow1, 0.0, 1.0);
  flow1 = smoothstep(0.10, 0.45, flow1);

  float flow2 = texture(u_flowTex2, v_st).r;
  flow2 = clamp(flow2, 0.0, 1.0);
  flow2 = smoothstep(0.10, 0.45, flow2);

  vec3 waterCol = u_flowColor1;
  vec3 lavaCol = u_flowColor2;

  col = mix(col, waterCol, 0.45 * flow1);
  col += 0.08 * flow1 * waterCol;

  col = mix(col, lavaCol, 0.55 * flow2);
  col += 0.12 * flow2 * lavaCol;

  FragColor = vec4(col, 1.0);
}
)GLSL";
