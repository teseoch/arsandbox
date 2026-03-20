#include "depth.hpp"

class Drop
{
public:
  float u, v;
  float life; // seconds

private:
  float du, dv; // velocity in UV
public:
  void step(const Depth &hf, float dt)
  {
    const float gravity = 0.25f;  // tune
    const float damping = 0.95f;  // tune
    const float speed_cap = 0.1f; // UV/sec

    auto [gx, gy] = hf.gradient_uv(u, v);

    // downhill direction is -grad
    du += -gravity * gx * dt;
    dv += -gravity * gy * dt;

    du *= std::pow(damping, dt * 60.0f);
    dv *= std::pow(damping, dt * 60.0f);

    float sp = std::sqrt(du * du + dv * dv);
    if (sp > speed_cap)
    {
      du *= speed_cap / sp;
      dv *= speed_cap / sp;
    }

    u += du * dt;
    v += dv * dt;

    // bounce / clamp at boundaries
    if (u < 0)
    {
      u = 0;
      du *= -0.5f;
    }
    if (u > 1)
    {
      u = 1;
      du *= -0.5f;
    }
    if (v < 0)
    {
      v = 0;
      dv *= -0.5f;
    }
    if (v > 1)
    {
      v = 1;
      dv *= -0.5f;
    }

    life -= dt;
  }

  inline bool isAlive() const { return life > 0; }
};
