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
#define MAT_COUNT  3
#define VARIANTS   2
#define FRAMES     4


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

vec3 sampleMatAnimated(int mat, vec2 st) {
  // stable UVs in sandbox space (NOT depth UV)
  vec2 uvT = st * u_tileScale;
  vec2 cell = floor(uvT);
  vec2 f    = fract(uvT);

  // choose a variant per tile cell
  float r = hash12(cell);
  int variant = int(floor(r * float(VARIANTS))); // 0..VARIANTS-1

  // animate frames (blend between consecutive frames)
  float fps = 2.0;               // tweak
  float tf  = u_time * fps;
  float a   = fract(tf);
  int f0    = int(mod(floor(tf), float(FRAMES)));
  int f1    = (f0 + 1) % FRAMES;

  int L0 = tileLayer(mat, variant, f0);
  int L1 = tileLayer(mat, variant, f1);

  vec3 c0 = texture(u_tiles, vec3(f, float(L0))).rgb;
  vec3 c1 = texture(u_tiles, vec3(f, float(L1))).rgb;
  return mix(c0, c1, a);
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
