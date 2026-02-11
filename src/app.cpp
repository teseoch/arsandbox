// sandbox_demo_single_file.cpp
//
// One-file, no-UI AR-sandbox-style prototype:
// - GLFW window + OpenGL (loads only the GL functions it uses via
// glfwGetProcAddress)
// - Renders ONE warped quad on black background
// - Synthetic uint16 "depth" generator (so you can develop on Unix without a
// sensor)
// - Calibrate projector quad (C) and depth UV quad (U) with 1-4 + arrows, Shift
// fine, Ctrl coarse
// - Colormap LUTs as 1D textures; cycle with M/N
// - Water overlay toggle W, sea level +/- , isolines toggle I
// - Optional gamepad polling (GLFW mapping) for a few knobs
//
// Build (Linux):
//   g++ sandbox_demo_single_file.cpp -O2 -std=c++17 -lglfw -ldl -lGL -o
//   sandbox_demo
//
// Build (macOS, Homebrew glfw):
//   clang++ sandbox_demo_single_file.cpp -O2 -std=c++17 -I/opt/homebrew/include
//   -L/opt/homebrew/lib -lglfw -framework OpenGL -framework Cocoa -framework
//   IOKit -framework CoreVideo -o sandbox_demo
//
// Notes:
// - Uses OpenGL 3.x style shaders. On macOS you must request core profile.
// - If you later swap synthetic depth for Kinect/RealSense, keep the depthTex
// upload path identical.

#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

// ----------------------------- Minimal GL loader -----------------------------
// We only load the GL entry points we actually use.
// This avoids GLAD/GLEW and keeps it single-file.

#if defined(__APPLE__)
#define APIENTRY
#include <OpenGL/gl3.h>
#else
#define APIENTRY
#include <GL/gl.h>
#endif

// Some platforms won't declare modern enums when including GL/gl.h
#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#endif
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_COMPILE_STATUS
#define GL_COMPILE_STATUS 0x8B81
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif
#ifndef GL_INFO_LOG_LENGTH
#define GL_INFO_LOG_LENGTH 0x8B84
#endif
#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#endif
#ifndef GL_STATIC_DRAW
#define GL_STATIC_DRAW 0x88E4
#endif
#ifndef GL_TEXTURE0
#define GL_TEXTURE0 0x84C0
#endif
#ifndef GL_TEXTURE1
#define GL_TEXTURE1 0x84C1
#endif
#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D 0x0DE1
#endif
#ifndef GL_TEXTURE_1D
#define GL_TEXTURE_1D 0x0DE0
#endif
#ifndef GL_R16UI
#define GL_R16UI 0x8234
#endif
#ifndef GL_RED_INTEGER
#define GL_RED_INTEGER 0x8D94
#endif
#ifndef GL_UNSIGNED_SHORT
// already exists in most headers
#define GL_UNSIGNED_SHORT 0x1403
#endif
#ifndef GL_RGB8
#define GL_RGB8 0x8051
#endif
#ifndef GL_RGB
#define GL_RGB 0x1907
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_NEAREST
#define GL_NEAREST 0x2600
#endif
#ifndef GL_LINEAR
#define GL_LINEAR 0x2601
#endif
#ifndef GL_TRIANGLE_FAN
#define GL_TRIANGLE_FAN 0x0006
#endif
#ifndef GL_FALSE
#define GL_FALSE 0
#endif
#ifndef GL_TRUE
#define GL_TRUE 1
#endif

// Function pointer types
using PFNGLCREATESHADERPROC = GLuint(APIENTRY *)(GLenum);
using PFNGLSHADERSOURCEPROC = void(APIENTRY *)(GLuint, GLsizei,
                                               const GLchar *const *,
                                               const GLint *);
using PFNGLCOMPILESHADERPROC = void(APIENTRY *)(GLuint);
using PFNGLGETSHADERIVPROC = void(APIENTRY *)(GLuint, GLenum, GLint *);
using PFNGLGETSHADERINFOLOGPROC = void(APIENTRY *)(GLuint, GLsizei, GLsizei *,
                                                   GLchar *);
using PFNGLCREATEPROGRAMPROC = GLuint(APIENTRY *)();
using PFNGLATTACHSHADERPROC = void(APIENTRY *)(GLuint, GLuint);
using PFNGLLINKPROGRAMPROC = void(APIENTRY *)(GLuint);
using PFNGLGETPROGRAMIVPROC = void(APIENTRY *)(GLuint, GLenum, GLint *);
using PFNGLGETPROGRAMINFOLOGPROC = void(APIENTRY *)(GLuint, GLsizei, GLsizei *,
                                                    GLchar *);
using PFNGLUSEPROGRAMPROC = void(APIENTRY *)(GLuint);
using PFNGLDELETESHADERPROC = void(APIENTRY *)(GLuint);
using PFNGLDELETEPROGRAMPROC = void(APIENTRY *)(GLuint);
using PFNGLGETUNIFORMLOCATIONPROC = GLint(APIENTRY *)(GLuint, const GLchar *);
using PFNGLUNIFORM1IPROC = void(APIENTRY *)(GLint, GLint);
using PFNGLUNIFORM1FPROC = void(APIENTRY *)(GLint, GLfloat);
using PFNGLUNIFORM2FPROC = void(APIENTRY *)(GLint, GLfloat, GLfloat);
using PFNGLUNIFORM2FVPROC = void(APIENTRY *)(GLint, GLsizei, const GLfloat *);
using PFNGLGENVERTEXARRAYSPROC = void(APIENTRY *)(GLsizei, GLuint *);
using PFNGLBINDVERTEXARRAYPROC = void(APIENTRY *)(GLuint);
using PFNGLGENTEXTURESPROC = void(APIENTRY *)(GLsizei, GLuint *);
using PFNGLBINDTEXTUREPROC = void(APIENTRY *)(GLenum, GLuint);
using PFNGLTEXIMAGE2DPROC = void(APIENTRY *)(GLenum, GLint, GLint, GLsizei,
                                             GLsizei, GLint, GLenum, GLenum,
                                             const void *);
using PFNGLTEXSUBIMAGE2DPROC = void(APIENTRY *)(GLenum, GLint, GLint, GLint,
                                                GLsizei, GLsizei, GLenum,
                                                GLenum, const void *);
using PFNGLTEXIMAGE1DPROC = void(APIENTRY *)(GLenum, GLint, GLint, GLsizei,
                                             GLint, GLenum, GLenum,
                                             const void *);
using PFNGLTEXSUBIMAGE1DPROC = void(APIENTRY *)(GLenum, GLint, GLint, GLsizei,
                                                GLenum, GLenum, const void *);
using PFNGLTEXPARAMETERIPROC = void(APIENTRY *)(GLenum, GLenum, GLint);
using PFNGLACTIVETEXTUREPROC = void(APIENTRY *)(GLenum);
using PFNGLDRAWARRAYSPROC = void(APIENTRY *)(GLenum, GLint, GLsizei);

static PFNGLCREATESHADERPROC glCreateShader_ = nullptr;
static PFNGLSHADERSOURCEPROC glShaderSource_ = nullptr;
static PFNGLCOMPILESHADERPROC glCompileShader_ = nullptr;
static PFNGLGETSHADERIVPROC glGetShaderiv_ = nullptr;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog_ = nullptr;
static PFNGLCREATEPROGRAMPROC glCreateProgram_ = nullptr;
static PFNGLATTACHSHADERPROC glAttachShader_ = nullptr;
static PFNGLLINKPROGRAMPROC glLinkProgram_ = nullptr;
static PFNGLGETPROGRAMIVPROC glGetProgramiv_ = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog_ = nullptr;
static PFNGLUSEPROGRAMPROC glUseProgram_ = nullptr;
static PFNGLDELETESHADERPROC glDeleteShader_ = nullptr;
static PFNGLDELETEPROGRAMPROC glDeleteProgram_ = nullptr;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation_ = nullptr;
static PFNGLUNIFORM1IPROC glUniform1i_ = nullptr;
static PFNGLUNIFORM1FPROC glUniform1f_ = nullptr;
static PFNGLUNIFORM2FPROC glUniform2f_ = nullptr;
static PFNGLUNIFORM2FVPROC glUniform2fv_ = nullptr;
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays_ = nullptr;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray_ = nullptr;
static PFNGLGENTEXTURESPROC glGenTextures_ = nullptr;
static PFNGLBINDTEXTUREPROC glBindTexture_ = nullptr;
static PFNGLTEXIMAGE2DPROC glTexImage2D_ = nullptr;
static PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D_ = nullptr;
static PFNGLTEXIMAGE1DPROC glTexImage1D_ = nullptr;
static PFNGLTEXSUBIMAGE1DPROC glTexSubImage1D_ = nullptr;
static PFNGLTEXPARAMETERIPROC glTexParameteri_ = nullptr;
static PFNGLACTIVETEXTUREPROC glActiveTexture_ = nullptr;
static PFNGLDRAWARRAYSPROC glDrawArrays_ = nullptr;

static void *getGLProc(const char *name) {
  return (void *)glfwGetProcAddress(name);
}

static bool loadGL() {
  glCreateShader_ = (PFNGLCREATESHADERPROC)getGLProc("glCreateShader");
  glShaderSource_ = (PFNGLSHADERSOURCEPROC)getGLProc("glShaderSource");
  glCompileShader_ = (PFNGLCOMPILESHADERPROC)getGLProc("glCompileShader");
  glGetShaderiv_ = (PFNGLGETSHADERIVPROC)getGLProc("glGetShaderiv");
  glGetShaderInfoLog_ =
      (PFNGLGETSHADERINFOLOGPROC)getGLProc("glGetShaderInfoLog");
  glCreateProgram_ = (PFNGLCREATEPROGRAMPROC)getGLProc("glCreateProgram");
  glAttachShader_ = (PFNGLATTACHSHADERPROC)getGLProc("glAttachShader");
  glLinkProgram_ = (PFNGLLINKPROGRAMPROC)getGLProc("glLinkProgram");
  glGetProgramiv_ = (PFNGLGETPROGRAMIVPROC)getGLProc("glGetProgramiv");
  glGetProgramInfoLog_ =
      (PFNGLGETPROGRAMINFOLOGPROC)getGLProc("glGetProgramInfoLog");
  glUseProgram_ = (PFNGLUSEPROGRAMPROC)getGLProc("glUseProgram");
  glDeleteShader_ = (PFNGLDELETESHADERPROC)getGLProc("glDeleteShader");
  glDeleteProgram_ = (PFNGLDELETEPROGRAMPROC)getGLProc("glDeleteProgram");
  glGetUniformLocation_ =
      (PFNGLGETUNIFORMLOCATIONPROC)getGLProc("glGetUniformLocation");
  glUniform1i_ = (PFNGLUNIFORM1IPROC)getGLProc("glUniform1i");
  glUniform1f_ = (PFNGLUNIFORM1FPROC)getGLProc("glUniform1f");
  glUniform2f_ = (PFNGLUNIFORM2FPROC)getGLProc("glUniform2f");
  glUniform2fv_ = (PFNGLUNIFORM2FVPROC)getGLProc("glUniform2fv");
  glGenVertexArrays_ = (PFNGLGENVERTEXARRAYSPROC)getGLProc("glGenVertexArrays");
  glBindVertexArray_ = (PFNGLBINDVERTEXARRAYPROC)getGLProc("glBindVertexArray");
  glGenTextures_ = (PFNGLGENTEXTURESPROC)getGLProc("glGenTextures");
  glBindTexture_ = (PFNGLBINDTEXTUREPROC)getGLProc("glBindTexture");
  glTexImage2D_ = (PFNGLTEXIMAGE2DPROC)getGLProc("glTexImage2D");
  glTexSubImage2D_ = (PFNGLTEXSUBIMAGE2DPROC)getGLProc("glTexSubImage2D");
  glTexImage1D_ = (PFNGLTEXIMAGE1DPROC)getGLProc("glTexImage1D");
  glTexSubImage1D_ = (PFNGLTEXSUBIMAGE1DPROC)getGLProc("glTexSubImage1D");
  glTexParameteri_ = (PFNGLTEXPARAMETERIPROC)getGLProc("glTexParameteri");
  glActiveTexture_ = (PFNGLACTIVETEXTUREPROC)getGLProc("glActiveTexture");
  glDrawArrays_ = (PFNGLDRAWARRAYSPROC)getGLProc("glDrawArrays");

  return glCreateShader_ && glShaderSource_ && glCompileShader_ &&
         glGetShaderiv_ && glGetShaderInfoLog_ && glCreateProgram_ &&
         glAttachShader_ && glLinkProgram_ && glGetProgramiv_ &&
         glGetProgramInfoLog_ && glUseProgram_ && glDeleteShader_ &&
         glDeleteProgram_ && glGetUniformLocation_ && glUniform1i_ &&
         glUniform1f_ && glUniform2f_ && glUniform2fv_ && glGenVertexArrays_ &&
         glBindVertexArray_ && glGenTextures_ && glBindTexture_ &&
         glTexImage2D_ && glTexSubImage2D_ && glTexImage1D_ &&
         glTexSubImage1D_ && glTexParameteri_ && glActiveTexture_ &&
         glDrawArrays_;
}

// ----------------------------- Tiny math types ------------------------------
struct Vec2 {
  float x = 0, y = 0;
};
struct Quad {
  Vec2 v[4];
};

static inline Vec2 clamp01(Vec2 p) {
  p.x = std::max(0.0f, std::min(1.0f, p.x));
  p.y = std::max(0.0f, std::min(1.0f, p.y));
  return p;
}

// ----------------------------- Controls / State -----------------------------
enum Mode { NONE, PROJ, UV };

struct ButtonEdge {
  bool prev = false;
  bool pressed(bool now) {
    bool r = (now && !prev);
    prev = now;
    return r;
  }
};

struct Controls {
  // Shader knobs
  float depthMinMm = 700.0f;
  float depthMaxMm = 1700.0f;
  float heightScale = 1.0f;
  float gamma = 1.0f;

  bool showWater = true;
  float seaLevel = 0.35f; // in height units after scaling (~0..1-ish)
  float shoreWidth = 0.03f;

  float isoStep = 0.0f; // 0 disables
  bool showIsolines = false;

  int colormapIndex = 0;
  int colormapCount = 4;

  bool freezeDepth = false;

  // Gamepad edges
  ButtonEdge aEdge, bEdge, xEdge, yEdge, lbEdge, rbEdge;
  bool gamepadPresent = false;

  void reset() {
    depthMinMm = 700.0f;
    depthMaxMm = 1700.0f;
    heightScale = 1.0f;
    gamma = 1.0f;
    showWater = true;
    seaLevel = 0.35f;
    shoreWidth = 0.03f;
    isoStep = 0.0f;
    showIsolines = false;
    colormapIndex = 0;
    freezeDepth = false;
  }
};

static Controls gCtl;

static Mode gMode = NONE;
static int gSelCorner = 0;

// Projector quad in *window pixel coords* (you warp this to match box corners)
static Quad gP;

// Depth UV quad in *normalized* [0,1] coords (you warp this to match depth ROI)
static Quad gU;

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

uniform usampler2D u_depthTex;   // GL_R16UI, mm
uniform sampler1D  u_colormapTex;

uniform vec2  u_depthUVQuad[4];  // normalized [0,1]
uniform float u_depthMinMm;
uniform float u_depthMaxMm;
uniform float u_heightScale;
uniform float u_gamma;

uniform int   u_showWater;
uniform float u_seaLevel;
uniform float u_shoreWidth;

uniform float u_isoStep;

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
  uint d16 = texture(u_depthTex, uv).r;
  float d = float(d16);
  if(d < 1.0){
    FragColor = vec4(0,0,0,1);
    return;
  }

  float dn = clamp((d - u_depthMinMm) / (u_depthMaxMm - u_depthMinMm), 0.0, 1.0);
  float h  = (1.0 - dn) * u_heightScale;

  float t = clamp(h, 0.0, 1.0);
  t = pow(t, u_gamma);
  vec3 col = texture(u_colormapTex, t).rgb;

  col = addIsolines(col, h, u_isoStep);

  if(u_showWater != 0){
    float w = smoothstep(u_seaLevel + u_shoreWidth, u_seaLevel - u_shoreWidth, h);
    vec3 water = vec3(0.05, 0.25, 0.85);
    // darken with "depth below sea"
    float deep = clamp((u_seaLevel - h) / max(u_seaLevel, 1e-3), 0.0, 1.0);
    water *= (0.7 + 0.3 * deep);
    col = mix(col, water, w);
  }

  FragColor = vec4(col, 1.0);
}
)GLSL";

// ----------------------------- GL Utils ------------------------------------
static GLuint compileShader(GLenum type, const char *src) {
  GLuint sh = glCreateShader_(type);
  glShaderSource_(sh, 1, &src, nullptr);
  glCompileShader_(sh);
  GLint ok = 0;
  glGetShaderiv_(sh, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    GLint len = 0;
    glGetShaderiv_(sh, GL_INFO_LOG_LENGTH, &len);
    std::string log;
    log.resize(std::max(1, len));
    GLsizei outLen = 0;
    glGetShaderInfoLog_(sh, (GLsizei)log.size(), &outLen, log.data());
    std::fprintf(stderr, "Shader compile error:\n%s\n", log.c_str());
    std::exit(1);
  }
  return sh;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
  GLuint prog = glCreateProgram_();
  glAttachShader_(prog, vs);
  glAttachShader_(prog, fs);
  glLinkProgram_(prog);
  GLint ok = 0;
  glGetProgramiv_(prog, GL_LINK_STATUS, &ok);
  if (!ok) {
    GLint len = 0;
    glGetProgramiv_(prog, GL_INFO_LOG_LENGTH, &len);
    std::string log;
    log.resize(std::max(1, len));
    GLsizei outLen = 0;
    glGetProgramInfoLog_(prog, (GLsizei)log.size(), &outLen, log.data());
    std::fprintf(stderr, "Program link error:\n%s\n", log.c_str());
    std::exit(1);
  }
  return prog;
}

// ----------------------------- Colormap LUTs -------------------------------
// Small procedural LUTs (256 entries). These are "map-like", not rainbow.
static std::vector<uint8_t> lut_normal(int L = 256) {
  std::vector<uint8_t> rgb(L * 3);
  auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
  auto put = [&](int i, float r, float g, float b) {
    rgb[3 * i + 0] = (uint8_t)std::lround(std::clamp(r, 0.0f, 1.0f) * 255.0f);
    rgb[3 * i + 1] = (uint8_t)std::lround(std::clamp(g, 0.0f, 1.0f) * 255.0f);
    rgb[3 * i + 2] = (uint8_t)std::lround(std::clamp(b, 0.0f, 1.0f) * 255.0f);
  };
  // water->green->brown->rock->snow
  struct Key {
    float t;
    float r, g, b;
  };
  Key keys[] = {
      {0.00f, 0.05f, 0.12f, 0.30f}, {0.18f, 0.05f, 0.25f, 0.70f},
      {0.28f, 0.10f, 0.55f, 0.25f}, {0.55f, 0.35f, 0.55f, 0.20f},
      {0.78f, 0.55f, 0.45f, 0.30f}, {0.90f, 0.75f, 0.75f, 0.75f},
      {1.00f, 0.95f, 0.95f, 0.95f},
  };
  for (int i = 0; i < L; i++) {
    float t = (float)i / (L - 1);
    int k = 0;
    while (k + 1 < (int)(sizeof(keys) / sizeof(keys[0])) && t > keys[k + 1].t)
      k++;
    Key a = keys[k],
        b = keys[std::min(k + 1, (int)(sizeof(keys) / sizeof(keys[0])) - 1)];
    float u = (b.t > a.t) ? (t - a.t) / (b.t - a.t) : 0.0f;
    put(i, lerp(a.r, b.r, u), lerp(a.g, b.g, u), lerp(a.b, b.b, u));
  }
  return rgb;
}

static std::vector<uint8_t> lut_tropical(int L = 256) {
  auto base = lut_normal(L);
  // shift towards more saturated greens and turquoise water
  for (int i = 0; i < L; i++) {
    float r = base[3 * i + 0] / 255.0f;
    float g = base[3 * i + 1] / 255.0f;
    float b = base[3 * i + 2] / 255.0f;
    g = std::min(1.0f, g * 1.15f);
    b = std::min(1.0f, b * 1.10f);
    r = std::max(0.0f, r * 0.95f);
    base[3 * i + 0] = (uint8_t)std::lround(r * 255);
    base[3 * i + 1] = (uint8_t)std::lround(g * 255);
    base[3 * i + 2] = (uint8_t)std::lround(b * 255);
  }
  return base;
}

static std::vector<uint8_t> lut_volcanic(int L = 256) {
  std::vector<uint8_t> rgb(L * 3);
  auto put = [&](int i, float r, float g, float b) {
    rgb[3 * i + 0] = (uint8_t)std::lround(std::clamp(r, 0.0f, 1.0f) * 255);
    rgb[3 * i + 1] = (uint8_t)std::lround(std::clamp(g, 0.0f, 1.0f) * 255);
    rgb[3 * i + 2] = (uint8_t)std::lround(std::clamp(b, 0.0f, 1.0f) * 255);
  };
  for (int i = 0; i < L; i++) {
    float t = (float)i / (L - 1);
    // dark basalt -> ember -> ash
    float r, g, b;
    if (t < 0.6f) {
      r = 0.05f + 0.25f * t;
      g = 0.05f + 0.10f * t;
      b = 0.06f + 0.08f * t;
    } else if (t < 0.85f) {
      float u = (t - 0.6f) / (0.25f);
      r = 0.20f + 0.70f * u;
      g = 0.10f + 0.25f * u;
      b = 0.08f + 0.10f * u;
    } else {
      float u = (t - 0.85f) / (0.15f);
      r = 0.90f + 0.08f * u;
      g = 0.35f + 0.55f * u;
      b = 0.18f + 0.70f * u;
    }
    put(i, r, g, b);
  }
  return rgb;
}

static std::vector<uint8_t> lut_ice(int L = 256) {
  std::vector<uint8_t> rgb(L * 3);
  auto put = [&](int i, float r, float g, float b) {
    rgb[3 * i + 0] = (uint8_t)std::lround(std::clamp(r, 0.0f, 1.0f) * 255);
    rgb[3 * i + 1] = (uint8_t)std::lround(std::clamp(g, 0.0f, 1.0f) * 255);
    rgb[3 * i + 2] = (uint8_t)std::lround(std::clamp(b, 0.0f, 1.0f) * 255);
  };
  for (int i = 0; i < L; i++) {
    float t = (float)i / (L - 1);
    float r = 0.02f + 0.90f * t;
    float g = 0.08f + 0.92f * t;
    float b = 0.12f + 0.95f * t;
    // add a bluish mid-tone
    if (t < 0.5f) {
      b = std::min(1.0f, b + 0.10f * (0.5f - t));
    }
    put(i, r, g, b);
  }
  return rgb;
}

static GLuint makeColormap1D(const std::vector<uint8_t> &rgb, int L = 256) {
  GLuint tex = 0;
  glGenTextures_(1, &tex);
  glBindTexture_(GL_TEXTURE_1D, tex);
  glTexImage1D_(GL_TEXTURE_1D, 0, GL_RGB8, L, 0, GL_RGB, GL_UNSIGNED_BYTE,
                rgb.data());
  glTexParameteri_(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri_(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri_(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  return tex;
}

// ----------------------------- Synthetic Depth ------------------------------
// Generates uint16 depth in millimeters, with a few smooth bumps that drift
// over time. This is NOT smoothing a real sensor; it's just a convenient
// development source.
static void generateDepthFrame(std::vector<uint16_t> &depth, int W, int H,
                               double t) {
  depth.resize((size_t)W * (size_t)H);
  auto idx = [&](int x, int y) { return (size_t)y * (size_t)W + (size_t)x; };

  // Base plane depth ~1100mm with moving hills.
  float base = 1100.0f;
  float a1 = 140.0f, a2 = 90.0f, a3 = 60.0f;

  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      float fx = (x + 0.5f) / (float)W;
      float fy = (y + 0.5f) / (float)H;

      float cx1 = 0.35f + 0.12f * (float)std::sin(0.7 * t);
      float cy1 = 0.45f + 0.10f * (float)std::cos(0.6 * t);
      float cx2 = 0.70f + 0.10f * (float)std::sin(0.9 * t + 1.0);
      float cy2 = 0.30f + 0.12f * (float)std::cos(0.8 * t + 0.3);
      float cx3 = 0.55f + 0.08f * (float)std::sin(1.1 * t + 2.0);
      float cy3 = 0.75f + 0.08f * (float)std::cos(1.0 * t + 0.8);

      auto gauss = [&](float fx, float fy, float cx, float cy, float s) {
        float dx = fx - cx, dy = fy - cy;
        return std::exp(-(dx * dx + dy * dy) / (2 * s * s));
      };

      float h = a1 * gauss(fx, fy, cx1, cy1, 0.12f) +
                a2 * gauss(fx, fy, cx2, cy2, 0.10f) +
                a3 * gauss(fx, fy, cx3, cy3, 0.08f);

      // a little ripple to mimic sand micro-variation
      float rip = 8.0f * std::sin(12.0f * fx + 9.0f * fy + (float)t) *
                  std::sin(10.0f * fy + (float)t * 0.7f);

      // Convert to depth: closer (smaller mm) where "hill" is higher
      float d = base - h - rip;

      // Clamp to valid-ish range
      d = std::max(300.0f, std::min(2200.0f, d));
      depth[idx(x, y)] = (uint16_t)std::lround(d);
    }
  }
}

// ----------------------------- Input Helpers --------------------------------
static float stepScale(int mods, float base) {
  if (mods & GLFW_MOD_SHIFT)
    return base * 0.2f; // fine
  if (mods & GLFW_MOD_CONTROL)
    return base * 5.0f; // coarse
  return base;
}

static void updateGamepad(Controls &c, float dt) {
  c.gamepadPresent = glfwJoystickPresent(GLFW_JOYSTICK_1) &&
                     glfwJoystickIsGamepad(GLFW_JOYSTICK_1);
  if (!c.gamepadPresent)
    return;

  GLFWgamepadstate s;
  if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &s))
    return;

  auto deadzone = [&](float x) { return (std::fabs(x) < 0.12f) ? 0.0f : x; };

  float ly = deadzone(s.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]); // -1 up, +1 down
  float ry = deadzone(s.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
  float rt =
      s.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]; // -1 released -> +1 pressed
  float lt = s.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
  float RT = std::clamp((rt + 1.0f) * 0.5f, 0.0f, 1.0f);
  float LT = std::clamp((lt + 1.0f) * 0.5f, 0.0f, 1.0f);

  // gamma with left Y
  c.gamma += (-ly) * (0.8f * dt);
  c.gamma = std::clamp(c.gamma, 0.2f, 3.0f);

  // sea level with triggers
  c.seaLevel += (RT - LT) * (0.6f * dt);
  c.seaLevel = std::clamp(c.seaLevel, 0.0f, 2.0f);

  // buttons
  bool A = (s.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS);
  bool B = (s.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS);
  bool X = (s.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS);
  bool Y = (s.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS);
  bool LB = (s.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS);
  bool RB = (s.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS);

  if (c.aEdge.pressed(A))
    c.colormapIndex = (c.colormapIndex + 1) % std::max(1, c.colormapCount);
  if (c.bEdge.pressed(B))
    c.showWater = !c.showWater;
  if (c.xEdge.pressed(X)) {
    c.showIsolines = !c.showIsolines;
    c.isoStep = c.showIsolines ? 0.10f : 0.0f;
  }
  if (c.yEdge.pressed(Y))
    c.reset();
  if (c.lbEdge.pressed(LB))
    c.colormapIndex =
        (c.colormapIndex - 1 + c.colormapCount) % std::max(1, c.colormapCount);
  if (c.rbEdge.pressed(RB))
    c.colormapIndex = (c.colormapIndex + 1) % std::max(1, c.colormapCount);
}

static void moveSelectedCorner(Quad &Q, int key, float step) {
  switch (key) {
  case GLFW_KEY_LEFT:
    Q.v[gSelCorner].x -= step;
    break;
  case GLFW_KEY_RIGHT:
    Q.v[gSelCorner].x += step;
    break;
  case GLFW_KEY_UP:
    Q.v[gSelCorner].y -= step;
    break;
  case GLFW_KEY_DOWN:
    Q.v[gSelCorner].y += step;
    break;
  default:
    break;
  }
}

// Keyboard: calibration + knobs
static void keyCallback(GLFWwindow *w, int key, int scancode, int action,
                        int mods) {
  if (action != GLFW_PRESS && action != GLFW_REPEAT)
    return;

  // mode toggles (press only)
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_C) {
      gMode = (gMode == PROJ) ? NONE : PROJ;
      return;
    }
    if (key == GLFW_KEY_U) {
      gMode = (gMode == UV) ? NONE : UV;
      return;
    }

    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) {
      gSelCorner = key - GLFW_KEY_1; // 0..3
      return;
    }

    // colormap cycle
    if (key == GLFW_KEY_M) {
      gCtl.colormapIndex =
          (gCtl.colormapIndex + 1) % std::max(1, gCtl.colormapCount);
      return;
    }
    if (key == GLFW_KEY_N) {
      gCtl.colormapIndex = (gCtl.colormapIndex - 1 + gCtl.colormapCount) %
                           std::max(1, gCtl.colormapCount);
      return;
    }

    // toggles
    if (key == GLFW_KEY_W) {
      gCtl.showWater = !gCtl.showWater;
      return;
    }
    if (key == GLFW_KEY_I) {
      gCtl.showIsolines = !gCtl.showIsolines;
      gCtl.isoStep = gCtl.showIsolines ? 0.10f : 0.0f;
      return;
    }
    if (key == GLFW_KEY_SPACE) {
      gCtl.freezeDepth = !gCtl.freezeDepth;
      return;
    }
    if (key == GLFW_KEY_R) {
      gCtl.reset();
      return;
    }
  }

  // calibration movement (press+repeat)
  if (gMode != NONE) {
    float step = 5.0f;
    if (mods & GLFW_MOD_SHIFT)
      step = 1.0f;
    if (mods & GLFW_MOD_CONTROL)
      step = 25.0f;

    if (gMode == PROJ) {
      moveSelectedCorner(gP, key, step);
    } else if (gMode == UV) {
      // UV uses normalized coordinates: scale step down
      float uvStep = step / 2000.0f; // tweak; feels ok for repeat
      moveSelectedCorner(gU, key, uvStep);
      gU.v[gSelCorner] = clamp01(gU.v[gSelCorner]);
    }
    return;
  }

  // parameter knobs (press+repeat)
  // Sea level +/- (nice "flood everything" effect with showWater on)
  if (key == GLFW_KEY_EQUAL) {
    gCtl.seaLevel += stepScale(mods, 0.01f);
    gCtl.seaLevel = std::clamp(gCtl.seaLevel, 0.0f, 2.0f);
    return;
  }
  if (key == GLFW_KEY_MINUS) {
    gCtl.seaLevel -= stepScale(mods, 0.01f);
    gCtl.seaLevel = std::clamp(gCtl.seaLevel, 0.0f, 2.0f);
    return;
  }

  // Depth min/max "chaos knobs"
  if (key == GLFW_KEY_LEFT_BRACKET) { // '['
    gCtl.depthMinMm -= stepScale(mods, 10.0f);
    return;
  }
  if (key == GLFW_KEY_RIGHT_BRACKET) { // ']'
    gCtl.depthMinMm += stepScale(mods, 10.0f);
    return;
  }
  if (key == GLFW_KEY_SEMICOLON) { // ';'
    gCtl.depthMaxMm -= stepScale(mods, 10.0f);
    return;
  }
  if (key == GLFW_KEY_APOSTROPHE) { // '''
    gCtl.depthMaxMm += stepScale(mods, 10.0f);
    return;
  }
  if (gCtl.depthMaxMm < gCtl.depthMinMm + 50.0f)
    gCtl.depthMaxMm = gCtl.depthMinMm + 50.0f;

  // Gamma
  if (key == GLFW_KEY_G) {
    // tap G cycles common gammas; hold with repeat will still just cycle
    // quickly
    if (action == GLFW_PRESS) {
      if (std::fabs(gCtl.gamma - 1.0f) < 1e-3f)
        gCtl.gamma = 0.8f;
      else if (std::fabs(gCtl.gamma - 0.8f) < 1e-3f)
        gCtl.gamma = 1.2f;
      else if (std::fabs(gCtl.gamma - 1.2f) < 1e-3f)
        gCtl.gamma = 1.6f;
      else
        gCtl.gamma = 1.0f;
    }
    return;
  }
}

// ----------------------------- Main -----------------------------------------
int main() {
  if (!glfwInit()) {
    std::fprintf(stderr, "Failed to init GLFW\n");
    return 1;
  }

#if defined(__APPLE__)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

  int winW = 1280, winH = 720;
  GLFWwindow *win =
      glfwCreateWindow(winW, winH, "AR Sandbox", nullptr, nullptr);
  if (!win) {
    std::fprintf(stderr, "Failed to create window\n");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  if (!loadGL()) {
    std::fprintf(stderr, "Failed to load required GL functions.\n");
    std::fprintf(stderr,
                 "Try updating drivers or adjusting GL version hints.\n");
    return 1;
  }

  glfwSetKeyCallback(win, keyCallback);

  // Initial projector quad covers whole window
  gP.v[0] = {0, 0};
  gP.v[1] = {(float)winW, 0};
  gP.v[2] = {(float)winW, (float)winH};
  gP.v[3] = {0, (float)winH};

  // Initial depth UV quad uses full depth image
  gU.v[0] = {0, 0};
  gU.v[1] = {1, 0};
  gU.v[2] = {1, 1};
  gU.v[3] = {0, 1};

  // Shaders
  GLuint vs = compileShader(GL_VERTEX_SHADER, kVS);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFS);
  GLuint prog = linkProgram(vs, fs);
  glDeleteShader_(vs);
  glDeleteShader_(fs);

  // Empty VAO (we use gl_VertexID)
  GLuint vao = 0;
  glGenVertexArrays_(1, &vao);
  glBindVertexArray_(vao);

  // Depth texture (synthetic)
  const int depthW = 320;
  const int depthH = 240;
  std::vector<uint16_t> depthCPU;
  generateDepthFrame(depthCPU, depthW, depthH, 0.0);

  GLuint depthTex = 0;
  glGenTextures_(1, &depthTex);
  glBindTexture_(GL_TEXTURE_2D, depthTex);
  glTexImage2D_(GL_TEXTURE_2D, 0, GL_R16UI, depthW, depthH, 0, GL_RED_INTEGER,
                GL_UNSIGNED_SHORT, depthCPU.data());
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Colormap LUTs (1D textures)
  std::vector<GLuint> lutTex;
  lutTex.push_back(makeColormap1D(lut_normal()));
  lutTex.push_back(makeColormap1D(lut_tropical()));
  lutTex.push_back(makeColormap1D(lut_volcanic()));
  lutTex.push_back(makeColormap1D(lut_ice()));
  gCtl.colormapCount = (int)lutTex.size();

  // Uniform locations
  glUseProgram_(prog);
  const GLint loc_projQuad = glGetUniformLocation_(prog, "u_projQuad");
  const GLint loc_screenSize = glGetUniformLocation_(prog, "u_screenSize");
  const GLint loc_depthUVQuad = glGetUniformLocation_(prog, "u_depthUVQuad");
  const GLint loc_depthMinMm = glGetUniformLocation_(prog, "u_depthMinMm");
  const GLint loc_depthMaxMm = glGetUniformLocation_(prog, "u_depthMaxMm");
  const GLint loc_heightScale = glGetUniformLocation_(prog, "u_heightScale");
  const GLint loc_gamma = glGetUniformLocation_(prog, "u_gamma");
  const GLint loc_showWater = glGetUniformLocation_(prog, "u_showWater");
  const GLint loc_seaLevel = glGetUniformLocation_(prog, "u_seaLevel");
  const GLint loc_shoreWidth = glGetUniformLocation_(prog, "u_shoreWidth");
  const GLint loc_isoStep = glGetUniformLocation_(prog, "u_isoStep");
  const GLint loc_depthSampler = glGetUniformLocation_(prog, "u_depthTex");
  const GLint loc_lutSampler = glGetUniformLocation_(prog, "u_colormapTex");

  // Bind samplers once
  glUniform1i_(loc_depthSampler, 0);
  glUniform1i_(loc_lutSampler, 1);

  double tPrev = glfwGetTime();
  double tSim = 0.0;

  while (!glfwWindowShouldClose(win)) {
    double tNow = glfwGetTime();
    float dt = (float)(tNow - tPrev);
    tPrev = tNow;

    // update window size (in case of resize)
    glfwGetFramebufferSize(win, &winW, &winH);
    glViewport(0, 0, winW, winH);

    // Optional gamepad controls (always safe; if no pad, no effect)
    updateGamepad(gCtl, dt);

    // Update synthetic depth
    if (!gCtl.freezeDepth) {
      tSim += dt;
      generateDepthFrame(depthCPU, depthW, depthH, tSim);
      glActiveTexture_(GL_TEXTURE0);
      glBindTexture_(GL_TEXTURE_2D, depthTex);
      glTexSubImage2D_(GL_TEXTURE_2D, 0, 0, 0, depthW, depthH, GL_RED_INTEGER,
                       GL_UNSIGNED_SHORT, depthCPU.data());
    }

    // Clear to black
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram_(prog);

    // upload uniforms
    // proj quad: 4 vec2 => 8 floats
    float P8[8] = {gP.v[0].x, gP.v[0].y, gP.v[1].x, gP.v[1].y,
                   gP.v[2].x, gP.v[2].y, gP.v[3].x, gP.v[3].y};
    float U8[8] = {gU.v[0].x, gU.v[0].y, gU.v[1].x, gU.v[1].y,
                   gU.v[2].x, gU.v[2].y, gU.v[3].x, gU.v[3].y};

    glUniform2fv_(loc_projQuad, 4, P8);
    glUniform2f_(loc_screenSize, (float)winW, (float)winH);
    glUniform2fv_(loc_depthUVQuad, 4, U8);

    glUniform1f_(loc_depthMinMm, gCtl.depthMinMm);
    glUniform1f_(loc_depthMaxMm, gCtl.depthMaxMm);
    glUniform1f_(loc_heightScale, gCtl.heightScale);
    glUniform1f_(loc_gamma, gCtl.gamma);

    glUniform1i_(loc_showWater, gCtl.showWater ? 1 : 0);
    glUniform1f_(loc_seaLevel, gCtl.seaLevel);
    glUniform1f_(loc_shoreWidth, gCtl.shoreWidth);
    glUniform1f_(loc_isoStep, gCtl.isoStep);

    // bind textures
    glActiveTexture_(GL_TEXTURE0);
    glBindTexture_(GL_TEXTURE_2D, depthTex);

    glActiveTexture_(GL_TEXTURE1);
    glBindTexture_(GL_TEXTURE_1D, lutTex[gCtl.colormapIndex]);

    // draw the warped quad
    glBindVertexArray_(vao);
    glDrawArrays_(GL_TRIANGLE_FAN, 0, 4);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  glDeleteProgram_(prog);
  glfwTerminate();
  return 0;
}

/*
Key cheat sheet:

Calibration:
  C : toggle projector-quad calibration (edit P)
  U : toggle depth-UV quad calibration (edit U)
  1 2 3 4 : select corner (TL, TR, BR, BL)
  Arrow keys : move selected corner
  Shift + arrows : fine move
  Ctrl  + arrows : coarse move

Visual / interaction:
  M / N : next / previous colormap
  W : toggle water overlay
  I : toggle isolines
  + / - : raise / lower sea level
  [ / ] : decrease / increase depthMinMm
  ; / ' : decrease / increase depthMaxMm
  Space : freeze/unfreeze depth
  R : reset parameters
  G : cycle a few gammas

Gamepad (if present, GLFW mapping):
  A: next colormap
  B: toggle water
  X: toggle isolines
  Y: reset
  Triggers: sea level up/down
  Left stick Y: gamma
  LB/RB: cycle colormap backward/forward
*/
