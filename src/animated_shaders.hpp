#pragma once

// ----------------------------- GLSL Shaders --------------------------------
static const char *kVSa = R"GLSL(
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

static const char *kFSa = R"GLSL(
#version 330 core
in vec2 v_st;
out vec4 FragColor;

uniform usampler2D u_depthTex;   // GL_R16UI, mm

uniform sampler2DArray u_tiles;  // GL_TEXTURE_2D_ARRAY
uniform float u_time;            // seconds
uniform float u_tileScale;       // e.g. 12..24 (tiles across sandbox)

uniform vec2  u_depthUVQuad[4];  // normalized [0,1]

uniform float u_depthMinMm;
uniform float u_depthMaxMm;
uniform float u_gamma;

// compile-time knobs
#define VARIANTS   3
#define FRAMES     2
#define MAT_COUNT  3
#define TILE_SIZE   64.0


float hash12(vec2 p) {
  return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

int tileLayer(int mat, int variant, int frame) {
  return ((mat * VARIANTS + variant) * FRAMES + frame);
}


// bilinear warp from quad coords to depth UV
vec2 warpUV(vec2 st){
  vec2 a = mix(u_depthUVQuad[0], u_depthUVQuad[1], st.x);
  vec2 b = mix(u_depthUVQuad[3], u_depthUVQuad[2], st.x);
  return mix(a, b, st.y);
}
  // Smoothstep-like ramp for nicer blending (removes grid)
float smooth01(float x) { return x*x*(3.0 - 2.0*x); } // Hermite

int variantForCell(vec2 cell) {
  float r = hash12(cell);
  return int(floor(r * float(VARIANTS))); // 0..VARIANTS-1
}

vec3 sampleTileLayer(int mat, int variant, int frame, vec2 local01) {
  // local01 should be in [0,1] but we allow wrap via fract()
  vec2 f = fract(local01);

  // snap to texel centers (helps with NEAREST / tiny tiles)
  vec2 wh = vec2(TILE_SIZE, TILE_SIZE);
  f = (floor(f * wh) + 0.5) / wh;

  int L = tileLayer(mat, variant, frame);
  return texture(u_tiles, vec3(f, float(L))).rgb;
}

vec3 sampleCellAnimated(int mat, vec2 cell, vec2 uvT) {
  int variant = variantForCell(cell);

  float fps = 2.0;
  float tf  = u_time * fps;
  float a   = fract(tf);
  int f0    = int(mod(floor(tf), float(FRAMES)));
  int f1    = (f0 + 1) % FRAMES;

  // local coords relative to THIS cell (can be negative; fract() wraps)
  vec2 local = uvT - cell;

  vec3 c0 = sampleTileLayer(mat, variant, f0, local);
  vec3 c1 = sampleTileLayer(mat, variant, f1, local);
  return mix(c0, c1, a);
}

vec3 sampleMatAnimated(int mat, vec2 st) {
  vec2 uvT  = st * u_tileScale;
  vec2 cell = floor(uvT);
  vec2 f    = fract(uvT);

  // blend weights across cell boundaries
  float fx = smooth01(f.x);
  float fy = smooth01(f.y);

  // 4 neighbor cells
  vec2 c00 = cell;
  vec2 c10 = cell + vec2(1.0, 0.0);
  vec2 c01 = cell + vec2(0.0, 1.0);
  vec2 c11 = cell + vec2(1.0, 1.0);

  vec3 s00 = sampleCellAnimated(mat, c00, uvT);
  vec3 s10 = sampleCellAnimated(mat, c10, uvT);
  vec3 s01 = sampleCellAnimated(mat, c01, uvT);
  vec3 s11 = sampleCellAnimated(mat, c11, uvT);

  vec3 sx0 = mix(s00, s10, fx);
  vec3 sx1 = mix(s01, s11, fx);
  return mix(sx0, sx1, fy);
}

vec3 sampleMatStatic(int mat, vec2 st) {
  vec2 uvT = st * u_tileScale;
  vec2 cell = floor(uvT);
  vec2 f    = fract(uvT);

  float r = hash12(cell);
  int variant = int(floor(r * float(VARIANTS)));

  int L0 = tileLayer(mat, variant, 0);
  return texture(u_tiles, vec3(f, float(L0))).rgb;
}

void materialWeights(float h, out float w0, out float w1, out float w2) {
  // 0=sand, 1=grass, 2=rock (example)
  float ws = smoothstep(0.00, 0.35, 1.0 - h);
  float wg = smoothstep(0.20, 0.60, h) * (1.0 - smoothstep(0.60, 0.85, h));
  float wr = smoothstep(0.55, 1.00, h);

  float sum = ws + wg + wr + 1e-6;
  w0 = ws / sum; w1 = wg / sum; w2 = wr / sum;
}

void main(){
  vec2 uv = warpUV(v_st);
  uint d16 = texture(u_depthTex, uv).r;
  float d = float(d16);
  if(d < 1.0){
    FragColor = vec4(0,0,0,1);
    return;
  }

  float dn = clamp((d - u_depthMinMm) / (u_depthMaxMm - u_depthMinMm), 0.0, 1.0);
  float h  = (1.0 - dn);

  float t = clamp(h, 0.0, 1.0);
  t = pow(t, u_gamma);

  float w0,w1,w2;
  materialWeights(t, w0,w1,w2);

  // animate only sand (looks like subtle shimmer); keep others static
  vec3 c0 = sampleMatAnimated(0, v_st);
  vec3 c1 = sampleMatAnimated(1, v_st);
  vec3 c2 = sampleMatAnimated(2, v_st);

  vec3 col = c0*w0 + c1*w1 + c2*w2;
  FragColor = vec4(col, 1.0);
}
)GLSL";
