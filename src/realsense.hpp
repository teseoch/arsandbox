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
    pipe.start(cfg);
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

    // outDepth = fs.get_depth_frame(); // Z16, aligned to color
    // outColor = fs.get_color_frame(); // BGR8

    // std::cout << "here" << dpth.get_profile().as<rs2::video_stream_profile>().width() << std::endl;
    // std::cout << dpth.get_profile().as<rs2::video_stream_profile>().height() << std::endl;

    const uint16_t *depthZ16 = (const uint16_t *)dpth.get_data(); // size w*h
    depth.depth.resize(w * h);
    std::memcpy(depth.depth.data(), depthZ16, w * h * sizeof(uint16_t));

    return fs.get_depth_frame() && fs.get_color_frame();
  }
};
