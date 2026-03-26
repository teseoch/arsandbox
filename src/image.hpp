#pragma once

#include <cstdint>
#include <string>
#include <vector>

std::vector<uint8_t> load_png_as_1d_texture(const std::string &filename, int &outWidth);

std::vector<uint8_t> load_png(const std::string &filename, int &outWidth, int &outHeight);
std::vector<uint8_t> load_png_rgba(const std::string &filename, int &outWidth, int &outHeight);