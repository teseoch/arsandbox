#include "depth.hpp"

class Drop {
public:
  float u, v;
  float life; // seconds

private:
  float du = 0, dv = 0; // velocity in UV
public:
  void step(const Depth &hf, float dt) {
    const float gravity = -0.35f; // tune
    const float damping = 0.98f;  // tune
    const float speed_cap = 0.08; // UV/sec

    auto [gx, gy] = hf.gradient_uv(u, v);
    float g = std::sqrt(gx * gx + gy * gy);

    if (g > 1e-5f) {
      gx /= g;
      gy /= g;
    }

    float force = std::max(g, 0.02f); // minimum drift on non-flat areas

    // downhill direction is -grad
    du = 1 * du - gravity * force * gx * dt;
    dv = 1 * dv - gravity * force * gy * dt;

    // du *= std::pow(damping, dt * 60.0f);
    // dv *= std::pow(damping, dt * 60.0f);

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

    life -= dt;
  }

  inline bool isAlive() const { return life > 0; }
};
