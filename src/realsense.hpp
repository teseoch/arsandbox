#include <librealsense2/rs.hpp>

#include "depth.hpp"

class RSGrabber
{
private:
	rs2::pipeline pipe;
	rs2::config cfg;
	rs2::align align_to_color = rs2::align(RS2_STREAM_COLOR);

	rs2::spatial_filter spatial;
	rs2::temporal_filter temporal;
	rs2::hole_filling_filter hole;
	rs2::decimation_filter decimate;

	bool started = false;

	const int scale = 1;
	int w = 640 / scale, h = 480 / scale;
	int fps = 30;

public:
	void start(Depth &depth)
	{
		cfg.enable_stream(RS2_STREAM_DEPTH, w * scale, h * scale, RS2_FORMAT_Z16, fps);
		cfg.enable_stream(RS2_STREAM_COLOR, w * scale, h * scale, RS2_FORMAT_BGR8, fps);
		rs2::pipeline_profile profile = pipe.start(cfg);
		started = true;

		spatial.set_option(RS2_OPTION_FILTER_MAGNITUDE, 4.0f);
		spatial.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.35f);
		spatial.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 30.0f);
		spatial.set_option(RS2_OPTION_HOLES_FILL, 1.0f);

		temporal.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.15f);
		temporal.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 40.0f);
		temporal.set_option(RS2_OPTION_HOLES_FILL, 0.0f);
		// temporal.set_option(RS2_OPTION_PERSIS, 0.0f);

		decimate.set_option(RS2_OPTION_FILTER_MAGNITUDE, 2.0f);

		auto depth_stream = profile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();

		rs2_intrinsics K = depth_stream.get_intrinsics();

		depth.cam_param.fx = K.fx;
		depth.cam_param.fy = K.fy;
		depth.cam_param.cx = K.ppx;
		depth.cam_param.cy = K.ppy;

		depth.h = h;
		depth.w = w;
		depth.depth.resize(w * h);
	}

	// Returns false if no frame (won't happen with wait_for_frames)
	bool grab(Depth &depth)
	{
		if (!started)
			return false;
		rs2::frameset fs = pipe.wait_for_frames();
		fs = align_to_color.process(fs);

		rs2::depth_frame dpth = fs.get_depth_frame();

		// rs2::disparity_transform depth_2_disp(true);
		// rs2::disparity_transform disp_2_dept(false);

		// dpth = decimate.process(dpth);
		// dpth = depth_2_disp.process(dpth);
		dpth = spatial.process(dpth);
		dpth = temporal.process(dpth);
		// dpth = disp_2_dept.process(dpth);
		dpth = hole.process(dpth);

		// Apply filters
		// dpth = spatial.process(dpth);
		// dpth = temporal.process(dpth);
		// dpth = hole.process(dpth);

		rs2::video_frame outColor = fs.get_color_frame(); // BGR8

		// std::cout << outColor.get_profile().format() << std::endl;

		// std::cout << "here" << dpth.get_profile().as<rs2::video_stream_profile>().width() << std::endl;
		// std::cout << dpth.get_profile().as<rs2::video_stream_profile>().height() << std::endl;

		const uint16_t *depthZ16 = (const uint16_t *)dpth.get_data(); // size w*h
		depth.depth.resize(w * h);

		const uint8_t *rgb = (const uint8_t *)outColor.get_data(); // size w*h*3
		depth.rgb.resize(w * h * 3);
		for (int i = 0; i < w * h; ++i)
		{
			depth.rgb[3 * i + 0] = rgb[3 * i + 2];
			depth.rgb[3 * i + 1] = rgb[3 * i + 1];
			depth.rgb[3 * i + 2] = rgb[3 * i + 0];

			depth.depth[i] = float(depthZ16[i]);
		}

		return fs.get_depth_frame() && fs.get_color_frame();
	}
};
