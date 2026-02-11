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

#include "image.hpp"
#include "utils.hpp"
#include <filesystem>

// ----------------------------- Tiny math types ------------------------------
struct Vec2 {
  float x = 0, y = 0;
};
struct Quad {
  Vec2 v[4];
};

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

static inline Vec2 clamp01(Vec2 p) {
  p.x = std::max(0.0f, std::min(1.0f, p.x));
  p.y = std::max(0.0f, std::min(1.0f, p.y));
  return p;
}

static Mode gMode = NONE;
static int gSelCorner = 0;

// Projector quad in *window pixel coords* (you warp this to match box corners)
static Quad gP;

// Depth UV quad in *normalized* [0,1] coords (you warp this to match depth ROI)
static Quad gU;

#include "input.hpp"
#include "shaders.hpp"

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
  const std::string folder = AR_IMAGE_FOLDER;
  // loop over *.png in folder images
  for (const auto &entry : std::filesystem::directory_iterator(folder)) {
    if (entry.is_regular_file() && entry.path().extension() == ".png") {
      int w = 0;
      auto data = load_png_as_1d_texture(entry.path().string(), w);
      if (!data.empty()) {
        lutTex.push_back(makeColormap1D(data, w));
        std::printf("Loaded colormap from %s\n", entry.path().string().c_str());
      }
    }
  }
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
