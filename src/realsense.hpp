#include <librealsense2/rs.hpp>

#include "depth.hpp"

class RSGrabber
{
private:
  rs2::pipeline pipe;
  rs2::config cfg;
  rs2::align align_to_color = rs2::align(RS2_STREAM_COLOR);
  bool started = false;

  int w = 640, h = 480;
  int fps = 30;

public:
  void start(Depth &depth)
  {
    cfg.enable_stream(RS2_STREAM_DEPTH, w, h, RS2_FORMAT_Z16, fps);
    cfg.enable_stream(RS2_STREAM_COLOR, w, h, RS2_FORMAT_BGR8, fps);
    pipe.start(cfg);
    started = true;

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
    // outDepth = fs.get_depth_frame(); // Z16, aligned to color
    // outColor = fs.get_color_frame(); // BGR8

    const uint16_t *depthZ16 = (const uint16_t *)fs.get_depth_frame().get_data(); // size w*h
    depth.depth.resize(w * h);
    std::memcpy(depth.depth.data(), depthZ16, w * h * sizeof(uint16_t));

    return fs.get_depth_frame() && fs.get_color_frame();
  }
};
