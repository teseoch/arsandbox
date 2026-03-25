#pragma once

#include "depth.hpp"
#include "types.hpp"

extern "C"
{
#include <apriltag.h>
#include <tag36h11.h>
}

struct DetectedTag
{
	int id = -1;
	float center_px_x = 0.0f;
	float center_px_y = 0.0f;
	std::array<Vec2, 4> corners_px{};
	Vec2 uv = {0.0f, 0.0f};
	float angle_rad = 0.0f;
	float decision_margin = 0.0f;
};

class TagDetector
{
public:
	TagDetector()
	{
		tf_ = tag36h11_create();
		td_ = apriltag_detector_create();
		apriltag_detector_add_family(td_, tf_);
		td_->quad_decimate = 1.0f;
		td_->quad_sigma = 0.8f;
		td_->refine_edges = 1;
		td_->nthreads = 2;
		td_->debug = 0;
	}

	~TagDetector()
	{
		if (td_)
			apriltag_detector_destroy(td_);
		if (tf_)
			tag36h11_destroy(tf_);
	}

	std::vector<DetectedTag> detect(const Depth &depth) const
	{
		std::vector<DetectedTag> out;

		if (!td_ || depth.rgb.empty() || depth.w <= 0 || depth.h <= 0)
			return out;

		std::vector<uint8_t> gray((size_t)depth.w * (size_t)depth.h);
		for (int i = 0; i < depth.w * depth.h; ++i)
		{
			const uint8_t r = depth.rgb[3 * i + 0];
			const uint8_t g = depth.rgb[3 * i + 1];
			const uint8_t b = depth.rgb[3 * i + 2];
			gray[i] = (uint8_t)std::clamp(int(0.299f * r + 0.587f * g + 0.114f * b), 0, 255);
		}
		// stbi_write_png("bla.png", depth.w, depth.h, 1, gray.data(), depth.w);

		image_u8_t img = {
			.width = depth.w,
			.height = depth.h,
			.stride = depth.w,
			.buf = gray.data()};

		zarray_t *detections = apriltag_detector_detect(td_, &img);
		const int n = zarray_size(detections);
		out.reserve((size_t)n);

		// std::cout << "n detections = " << n << std::endl;

		for (int i = 0; i < n; ++i)
		{
			apriltag_detection_t *det = nullptr;
			zarray_get(detections, i, &det);
			if (!det)
				continue;

			DetectedTag t;
			t.id = det->id;
			t.center_px_x = (float)det->c[0];
			t.center_px_y = (float)det->c[1];
			t.decision_margin = (float)det->decision_margin;

			for (int k = 0; k < 4; ++k)
			{
				t.corners_px[(size_t)k] = {(float)det->p[k][0], (float)det->p[k][1]};
			}

			const float u = std::clamp(t.center_px_x / std::max(1, depth.w - 1), 0.0f, 1.0f);
			const float v = std::clamp(t.center_px_y / std::max(1, depth.h - 1), 0.0f, 1.0f);
			t.uv = {u, v};

			const float ex = (float)(det->p[1][0] - det->p[0][0]);
			const float ey = (float)(det->p[1][1] - det->p[0][1]);
			t.angle_rad = std::atan2(ey, ex);
			out.push_back(t);
		}

		apriltag_detections_destroy(detections);
		return out;
	}

private:
	apriltag_family_t *tf_ = nullptr;
	apriltag_detector_t *td_ = nullptr;
};