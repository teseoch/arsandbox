#pragma once

#include <algorithm>
#include <vector>

class FlowMap {
public:
  int w = 0, h = 0;
  std::vector<float> flow;

  void resize(int w_, int h_) {
    w = w_;
    h = h_;
    flow.assign(w * h, 0.0f);
  }

  void clear() { std::fill(flow.begin(), flow.end(), 0.0f); }

  void decay(float factor) {
    for (float &v : flow)
      v *= factor;
  }

  void splat(float u, float v, float amount) {
    if (w <= 0 || h <= 0)
      return;

    int cx = std::clamp(int(u * float(w - 1)), 0, w - 1);
    int cy = std::clamp(int(v * float(h - 1)), 0, h - 1);

    static const float K[3][3] = {
        {1.0f, 2.0f, 1.0f},
        {2.0f, 4.0f, 2.0f},
        {1.0f, 2.0f, 1.0f},
    };

    for (int j = -1; j <= 1; ++j) {
      for (int i = -1; i <= 1; ++i) {
        int x = std::clamp(cx + i, 0, w - 1);
        int y = std::clamp(cy + j, 0, h - 1);
        flow[y * w + x] += amount * (K[j + 1][i + 1] / 16.0f);
      }
    }
  }

  void diffuse_once() {
    if (w <= 2 || h <= 2)
      return;
    std::vector<float> tmp = flow;
    for (int y = 1; y < h - 1; ++y) {
      for (int x = 1; x < w - 1; ++x) {
        float c = tmp[y * w + x];
        float n = tmp[(y - 1) * w + x];
        float s = tmp[(y + 1) * w + x];
        float e = tmp[y * w + (x + 1)];
        float wv = tmp[y * w + (x - 1)];
        flow[y * w + x] = 0.86f * c + 0.035f * (n + s + e + wv);
      }
    }
  }
};
