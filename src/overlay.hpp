#pragma once

#include "utils.hpp"

#include <cstdint>
#include <vector>

struct OverlaySprite {
  float st_x;       // [0,1]
  float st_y;       // [0,1]
  float radius_px;  // pixels
  float r, g, b, a; // [0,1]
};

struct OverlayRenderer {
  GLuint vao = 0;
  GLuint quadVBO = 0;
  GLuint instVBO = 0;
};

OverlayRenderer overlayInit();

void overlayDraw(
    const OverlayRenderer &R, GLuint overlayProgram, int screenW, int screenH,
    const float projQuadPx[8], // [x0,y0, x1,y1, x2,y2, x3,y3] TL,TR,BR,BL
    const std::vector<OverlaySprite> &sprites);

GLuint createOverlayProgram();
