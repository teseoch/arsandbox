#pragma once

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
