#include "image.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void forceWrapEdgesRGB(std::vector<uint8_t> &data, int w, int h)
{
  auto px = [&](int x, int y) -> uint8_t *
  { return &data[3 * (y * w + x)]; };

  // last column = first column
  for (int y = 0; y < h; ++y)
  {
    std::memcpy(px(w - 1, y), px(0, y), 3);
  }
  // last row = first row
  for (int x = 0; x < w; ++x)
  {
    std::memcpy(px(x, h - 1), px(x, 0), 3);
  }

  std::memcpy(px(w - 1, h - 1), px(0, 0), 3);
}

// Loads an image file and returns a 1D RGB byte array suitable for a 1D
// texture. On success:  returns vector of size (width * 3) in RGB order. On
// failure:  returns empty vector. Also returns the image width via outWidth.

std::vector<uint8_t> load_png_as_1d_texture(const std::string &filename,
                                            int &outWidth)
{
  int w = 0, h = 0, channels = 0;

  // Force 3 channels (RGB) regardless of input format
  stbi_uc *data = stbi_load(filename.c_str(), &w, &h, &channels, 3);

  if (!data)
  {
    std::cerr << "stb_image failed to load: " << filename << "\n";
    outWidth = 0;
    return {};
  }

  // We only care about width for a 1D colormap.
  outWidth = std::max(w, h);

  // Allocate output: width * 3 bytes (RGB)
  std::vector<uint8_t> tex1d;
  tex1d.resize(outWidth * 3);
  // Copy the first row into a 1D texture
  // (you can change this later if you want to average rows)
  for (int x = 0; x < outWidth; ++x)
  {
    if (w >= h)
    {
      tex1d[3 * x + 0] = data[3 * x + 0]; // R
      tex1d[3 * x + 1] = data[3 * x + 1]; // G
      tex1d[3 * x + 2] = data[3 * x + 2]; // B
    }
    else
    {
      tex1d[3 * x + 0] = data[3 * (w * x) + 0]; // R
      tex1d[3 * x + 1] = data[3 * (w * x) + 1]; // G
      tex1d[3 * x + 2] = data[3 * (w * x) + 2]; // B
    }
  }

  stbi_image_free(data);
  return tex1d;
}

std::vector<uint8_t> load_png(const std::string &filename, int &outWidth,
                              int &outHeight)
{
  int w = 0, h = 0, channels = 0;

  // Force 3 channels (RGB) regardless of input format
  stbi_uc *data = stbi_load(filename.c_str(), &w, &h, &channels, 3);

  if (!data)
  {
    std::cerr << "stb_image failed to load: " << filename << "\n";
    outWidth = 0;
    outHeight = 0;
    return {};
  }

  // We only care about width for a 1D colormap.
  outWidth = w;
  outHeight = h;

  // Allocate output: width * 3 bytes (RGB)
  std::vector<uint8_t> tex;
  tex.resize(outWidth * outHeight * 3);
  // Copy the first row into a 1D texture
  // (you can change this later if you want to average rows)
  for (int x = 0; x < outWidth; ++x)
  {
    for (int y = 0; y < outHeight; ++y)
    {
      int idx = 3 * (y * w + x);
      tex[idx + 0] = data[idx + 0]; // R
      tex[idx + 1] = data[idx + 1]; // G
      tex[idx + 2] = data[idx + 2]; // B
    }
  }

  stbi_image_free(data);

  forceWrapEdgesRGB(tex, outWidth, outHeight);
  return tex;
}