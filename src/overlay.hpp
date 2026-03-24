#pragma once

#include "utils.hpp"

#include <cstdint>
#include <vector>

struct OverlaySprite {
  float st_x;       // [0,1]
  float st_y;       // [0,1]
  float radius_px;  // pixels
  float r, g, b, a; // [0,1]

  // Optional textured-sprite support. Existing aggregate initializers that only
  // provide the first 7 floats will keep using the default circle sprite mode.
  float kind = 0.0f; // 0 = soft circle, 1 = textured sprite
  float uv0_x = 0.0f;
  float uv0_y = 0.0f;
  float uv1_x = 1.0f;
  float uv1_y = 1.0f;
};

struct OverlayRenderer {
  GLuint vao = 0;
  GLuint quadVBO = 0;
  GLuint instVBO = 0;
  GLuint spriteTex = 0;
};

OverlayRenderer overlayInit();

void overlaySetSpriteTexture(OverlayRenderer &R, GLuint tex);
GLuint overlayCreateRGBA8Texture(const unsigned char *rgba, int w, int h);

void overlayDraw(
    const OverlayRenderer &R, GLuint overlayProgram, int screenW, int screenH,
    const float projQuadPx[8], // [x0,y0, x1,y1, x2,y2, x3,y3] TL,TR,BR,BL
    const std::vector<OverlaySprite> &sprites);

GLuint createOverlayProgram();
