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

void Depth::newDepth()
{
	depth_history.push_back(depth);

	if (depth_history.size() == 1)
	{
		depth_smooth = depth;
		return;
	}

	if (depth_history.size() > MAX_HISTORY)
	{
		for (size_t i = 0; i < depth.size(); i++)
			depth_smooth[i] += (depth_history.back()[i] - depth_history.front()[i]) / MAX_HISTORY;

		depth_history.pop_front();
	}
	else
	{
		for (size_t i = 0; i < depth.size(); i++)
			depth_smooth[i] = (depth_smooth[i] * (depth_history.size() - 1) + depth[i]) / depth_history.size();
	}
}

void Depth::apply_ray_to_plane_correction()
{
	if (cam_param.fx <= 0.0f || cam_param.fy <= 0.0f)
		return;

	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			const int i = y * w + x;
			const float d = depth[i];
			if (d <= 0.0f)
				continue;

			const float nx = (float(x) - cam_param.cx) / cam_param.fx;
			const float ny = (float(y) - cam_param.cy) / cam_param.fy;
			const float corr = 1.0f / std::sqrt(1.0f + nx * nx + ny * ny);

			depth[i] = d * corr;
		}
	}
}

void Depth::save_png(const std::string &filename) const
{
	if (depth.empty())
		return;

	std::string txtPath = to_txt_path(filename);
	if (FILE *fp = std::fopen(txtPath.c_str(), "w"))
	{
		for (int y = 0; y < h; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				if (x > 0)
					std::fputc(' ', fp);
				std::fprintf(fp, "%lf", depth[(size_t)y * (size_t)w + (size_t)x]);
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
