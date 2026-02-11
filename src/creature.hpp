#pragma once

#include "depth.hpp"

#include <cmath>
#include <iostream>

class Creature {
public:
  float u, v;
  float h0;
  float dir; // +1 or -1 (cw/ccw)

  const float tangential_speed = 0.08f; // UV/sec
  const float contour_gain = 3.0f;      // how strongly it returns to isoline

  void step(const Depth &hf, float dt) {
    auto [gx, gy] = hf.gradient_uv(u, v);
    float gnorm = std::sqrt(gx * gx + gy * gy) + 1e-6f;

    // tangent = rotate(grad) by 90 deg
    float tx = dir * (-gy / gnorm);
    float ty = dir * (gx / gnorm);

    float z = hf.sample_bilinear(u, v);
    float err = (z - h0); // >0 means above target height

    // correction toward isoline: move along -grad if above, +grad if below
    float cx = -(err) * (gx / (gnorm * gnorm));
    float cy = -(err) * (gy / (gnorm * gnorm));

    u += (tangential_speed * tx + contour_gain * cx) * dt;
    v += (tangential_speed * ty + contour_gain * cy) * dt;

    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);
  }
};
