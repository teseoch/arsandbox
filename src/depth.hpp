#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

class Depth {
public:
  int w, h;
  std::vector<uint16_t> depth; // millimeters, size w*h
  float depthMinMm, depthMaxMm;

  inline float height(int xi, int yi) const {
    const float d = depth[yi * w + xi];
    // std::cout << "height at xi=" << xi << " yi=" << yi << " => d=" << d <<
    // "\n";
    return std::clamp((d - depthMinMm) / (depthMaxMm - depthMinMm), 0.0f, 1.0f);
  }

  inline float sample_bilinear(float u, float v) const {
    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);
    float x = u * (w - 1);
    float y = v * (h - 1);
    int x0 = (int)std::floor(x), y0 = (int)std::floor(y);
    int x1 = std::min(x0 + 1, w - 1);
    int y1 = std::min(y0 + 1, h - 1);
    float tx = x - x0, ty = y - y0;

    // std::cout << "sample_bilinear at u=" << u << " v=" << v << " => x=" << x
    //           << " y=" << y << " x0=" << x0 << " y0=" << y0 << " x1=" << x1
    //           << " y1=" << y1 << " tx=" << tx << " ty=" << ty << "\n";

    float z00 = height(x0, y0);
    float z10 = height(x1, y0);
    float z01 = height(x0, y1);
    float z11 = height(x1, y1);

    // std::cout << "  corners: z00=" << z00 << " z10=" << z10 << " z01=" << z01
    //           << " z11=" << z11 << "\n";

    float z0 = z00 * (1 - tx) + z10 * tx;
    float z1 = z01 * (1 - tx) + z11 * tx;
    return z0 * (1 - ty) + z1 * ty;
  }

  inline std::pair<float, float> gradient_uv(float u, float v) const {
    // finite differences in UV (choose eps as ~1 pixel in UV)
    float du = 1.0f / std::max(1, w - 1);
    float dv = 1.0f / std::max(1, h - 1);
    float zx1 = sample_bilinear(u + du, v);
    float zx0 = sample_bilinear(u - du, v);
    float zy1 = sample_bilinear(u, v + dv);
    float zy0 = sample_bilinear(u, v - dv);
    float gx = (zx1 - zx0) / (2 * du);
    float gy = (zy1 - zy0) / (2 * dv);
    return {gx, gy}; // ∂z/∂u, ∂z/∂v
  }

  void save_png(const std::string &filename) const;
};
