#pragma once

#include "depth.hpp"
#include <deque>
#include <utility>

class Drop {
public:
  float u = 0.0f, v = 0.0f;
  float life = 0.0f; // seconds
  std::deque<std::pair<float, float>> trail;
  int max_trail = 30;

private:
  float du = 0.0f, dv = 0.0f; // velocity in UV
public:
  void step(const Depth &hf, float dt) {
    if (life <= 0.0f)
      return;

    const float gravity = 0.20f;
    const float damping = 0.992f;
    const float speed_cap = 0.06f;

    auto [gx, gy] = hf.gradient_uv(u, v);
    float g = std::sqrt(gx * gx + gy * gy);

    if (g < 0.03f) {
      gx = 0.0f;
      gy = 0.0f;
      g = 0.0f;
    }

    du += -gravity * gx * dt;
    dv += -gravity * gy * dt;

    du *= std::pow(damping, dt * 60.0f);
    dv *= std::pow(damping, dt * 60.0f);

    float sp = std::sqrt(du * du + dv * dv);
    if (sp > speed_cap) {
      du *= speed_cap / sp;
      dv *= speed_cap / sp;
    }

    u += du * dt;
    v += dv * dt;

    // bounce / clamp at boundaries
    if (u < 0) {
      u = 0;
      du *= -0.5f;
    }
    if (u > 1) {
      u = 1;
      du *= -0.5f;
    }
    if (v < 0) {
      v = 0;
      dv *= -0.5f;
    }
    if (v > 1) {
      v = 1;
      dv *= -0.5f;
    }

    trail.emplace_back(u, v);
    while ((int)trail.size() > max_trail)
      trail.pop_front();

    life -= dt;
    if (life <= 0.0f)
      trail.clear();
  }

  inline bool isAlive() const { return life > 0; }

  inline void reset(float u0, float v0, float life0) {
    u = u0;
    v = v0;
    life = life0;
    du = 0.0f;
    dv = 0.0f;
    trail.clear();
    trail.emplace_back(u, v);
  }
};
