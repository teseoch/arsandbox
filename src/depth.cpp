#include "depth.hpp"

#include "utils.hpp"

static std::string to_pgm_path(const std::string &filename)
{
  if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".png")
  {
    return filename.substr(0, filename.size() - 4) + ".pgm";
  }
  return filename + ".pgm";
}

static std::string to_txt_path(const std::string &filename)
{
  if (filename.size() >= 4 && filename.substr(filename.size() - 4) == ".png")
  {
    return filename.substr(0, filename.size() - 4) + ".txt";
  }
  return filename + ".txt";
}

void Depth::save_png(const std::string &filename) const
{
  if (depth.empty())
    return;

  std::string pgmPath = to_pgm_path(filename);
  if (FILE *fp = std::fopen(pgmPath.c_str(), "wb"))
  {
    std::fprintf(fp, "P5\n%d %d\n65535\n", w, h);
    for (uint16_t d : depth)
    {
      std::fputc((d >> 8) & 0xFF, fp);
      std::fputc(d & 0xFF, fp);
    }
    std::fclose(fp);
  }

  std::string txtPath = to_txt_path(filename);
  if (FILE *fp = std::fopen(txtPath.c_str(), "w"))
  {
    for (int y = 0; y < h; ++y)
    {
      for (int x = 0; x < w; ++x)
      {
        if (x > 0)
          std::fputc(' ', fp);
        std::fprintf(fp, "%u", depth[(size_t)y * (size_t)w + (size_t)x]);
      }
      std::fputc('\n', fp);
    }
    std::fclose(fp);
  }

  // uint16_t mn = *std::min_element(depth.begin(), depth.end());
  // uint16_t mx = *std::max_element(depth.begin(), depth.end());
  // float scale = (mx > mn) ? 255.0f / (mx - mn) : 0.0f;
  // std::vector<uint8_t> gray(depth.size());
  // for (size_t i = 0; i < depth.size(); i++)
  //   gray[i] = static_cast<uint8_t>((depth[i] - mn) * scale);
  // stbi_write_png(filename.c_str(), w, h, 1, gray.data(), w);

  stbi_write_png(filename.c_str(), w, h, 3, rgb.data(), w * 3);
}
