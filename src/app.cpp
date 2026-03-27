// sandbox_demo_single_file.cpp
//
// One-file, no-UI AR-sandbox-style prototype:
// - GLFW window + OpenGL (loads only the GL functions it uses via
// glfwGetProcAddress)
// - Renders ONE warped quad on black background
// - Synthetic uint16 "depth" generator (so you can develop on Unix without a
// sensor)
// - Calibrate projector quad (C) and depth UV quad (U) with 1-4 + arrows, Shift
// fine, Ctrl coarse
// - Colormap LUTs as 1D textures; cycle with M/N
// - Water overlay toggle W, sea level +/- , isolines toggle I
// - Optional gamepad polling (GLFW mapping) for a few knobs
//
// Build (Linux):
//   g++ sandbox_demo_single_file.cpp -O2 -std=c++17 -lglfw -ldl -lGL -o
//   sandbox_demo
//
// Build (macOS, Homebrew glfw):
//   clang++ sandbox_demo_single_file.cpp -O2 -std=c++17 -I/opt/homebrew/include
//   -L/opt/homebrew/lib -lglfw -framework OpenGL -framework Cocoa -framework
//   IOKit -framework CoreVideo -o sandbox_demo
//
// Notes:
// - Uses OpenGL 3.x style shaders. On macOS you must request core profile.
// - If you later swap synthetic depth for Kinect/RealSense, keep the depthTex
// upload path identical.

#include "creature.hpp"
#include "depth.hpp"
#include "drop.hpp"
#include "image.hpp"
#include "overlay.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "TagDetector.hpp"
#include "input.hpp"
#include "Simulation.hpp"
#include "CMap.hpp"

#include "animated_shaders.hpp"
#include "c_map_shaders.hpp"

#ifdef SANDBOX_WITH_REALSENSE
#include "realsense.hpp"
#endif

#include <array>
#include <filesystem>
#include <memory>

const static bool fullscreen = true;

const static int VARIANTS = 3;
const static int FRAMES = 2;
const static int MAT_COUNT = 3;

const static int CREATURE_SHEET_COLS = 8;
const static int CREATURE_SHEET_ROWS = 3; // row 0 = walk, row 1 = panic, row 2 = dead
const static float CREATURE_ANIM_FPS = 8.0f;

const static int tileW = 64;
const static int tileH = 64;

void render_particles(
	const std::vector<Drop> &parts,
	const std::array<float, 3> &col,
	const std::array<float, 2> &sizes,
	std::vector<OverlaySprite> &sprites)
{
	for (const auto &d : parts)
	{
		for (size_t i = 0; i < d.trail.size(); ++i)
		{
			float t = float(i + 1) / float(d.trail.size());
			const auto &[u, v] = d.trail[i];

			// Push trail color slightly toward cyan/white so it still reads as
			// water.
			float rr = 0.65f * col[0] + 0.35f * 0.75f;
			float rg = 0.65f * col[1] + 0.35f * 0.95f;
			float rb = 0.65f * col[2] + 0.35f * 1.00f;

			sprites.push_back(
				{u, v, sizes[0] * t, 0.0f, 0.0f, rr, rg, rb, 0.02f + 0.18f * t});
		}
		float rr = 0.50f * col[0] + 0.50f * 0.75f;
		float rg = 0.50f * col[1] + 0.50f * 0.95f;
		float rb = 0.50f * col[2] + 0.50f * 1.00f;

		sprites.push_back({d.u, d.v, sizes[1], 0.0f, 0.0f, rr, rg, rb, 0.9f});
	}
}

// ----------------------------- Main -----------------------------------------
int main()
{
	const std::string folder = AR_IMAGE_FOLDER;

	std::srand(std::time(nullptr));
	// std::srand(42); // fixed seed for repeatable testing

	if (!glfwInit())
	{
		std::fprintf(stderr, "Failed to init GLFW\n");
		return 1;
	}

#if defined(__APPLE__)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	GLFWwindow *win;
	int winW, winH;
	if (fullscreen)
	{
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);

		winW = mode->width;
		winH = mode->height;

		win = glfwCreateWindow(winW, winH, "AR Sandbox", monitor, nullptr);
	}
	else
	{
		winW = 1280;
		winH = 720;
		win = glfwCreateWindow(winW, winH, "AR Sandbox", nullptr, nullptr);
	}
	if (!win)
	{
		std::fprintf(stderr, "Failed to create window\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(win);
	glfwSwapInterval(1);

	if (!loadGL())
	{
		std::fprintf(stderr, "Failed to load required GL functions.\n");
		std::fprintf(stderr,
					 "Try updating drivers or adjusting GL version hints.\n");
		return 1;
	}

	gCtl.reset();

	glfwSetKeyCallback(win, keyCallback);

	// once
	OverlayRenderer overlay;

	int creatureSheetW = 0, creatureSheetH = 0;
	{
		{
			std::vector<uint8_t> creatureRGBA =
				load_png_rgba(folder + "/creatures/goat-sheet.png", creatureSheetW, creatureSheetH);
			if (!creatureRGBA.empty() && creatureSheetW > 0 && creatureSheetH > 0)
			{
				overlay.createRGBA8Texture(creatureRGBA.data(), creatureSheetW, creatureSheetH, 0);
			}
			else
			{
				std::cerr << "Failed to load creature sprite texture: "
						  << (folder + "/creatures/goat-sheet.png") << std::endl;
			}
		}

		{
			std::vector<uint8_t> creatureRGBA =
				load_png_rgba(folder + "/creatures/pig-sheet.png", creatureSheetW, creatureSheetH);
			if (!creatureRGBA.empty() && creatureSheetW > 0 && creatureSheetH > 0)
			{
				overlay.createRGBA8Texture(creatureRGBA.data(), creatureSheetW, creatureSheetH, 1);
			}
			else
			{
				std::cerr << "Failed to load creature sprite texture: "
						  << (folder + "/creatures/pig-sheet.png") << std::endl;
			}
		}
	}

	TagDetector tagDetector;

	// Initial projector quad covers whole windows
	if (std::filesystem::exists("calib.txt"))
	{
		std::ifstream logIn("calib.txt");
		if (logIn)
		{
			for (int i = 0; i < 4; i++)
				logIn >> gP.v[i].x >> gP.v[i].y;
			for (int i = 0; i < 4; i++)
				logIn >> gU.v[i].x >> gU.v[i].y;
		}
	}
	else
	{
		gP.v[0] = {0, 0};
		gP.v[1] = {(float)winW, 0};
		gP.v[2] = {(float)winW, (float)winH};
		gP.v[3] = {0, (float)winH};
		// Initial depth UV quad uses full depth image
		gU.v[0] = {0, 0};
		gU.v[1] = {1, 0};
		gU.v[2] = {1, 1};
		gU.v[3] = {0, 1};
	}
	gCtl.depth.depthMinMm = 1875.0f;
	gCtl.depth.depthMaxMm = 1950.0f;

	// Shaders
	GLuint vs = compileShader(GL_VERTEX_SHADER, kVS);
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFS);

	GLuint prog = linkProgram(vs, fs);
	glDeleteShader_(vs);
	glDeleteShader_(fs);

	GLuint overlayProgram = overlay.createProgram();

	GLuint vao = 0;
	glGenVertexArrays_(1, &vao);
	glBindVertexArray_(vao);

	// Depth texture (synthetic)
#ifdef SANDBOX_WITH_REALSENSE
	RSGrabber realsense;
	realsense.start(gCtl.depth);
#else
	// generateDepthFrame(gCtl.depth, 0.0);
	static const std::string depth_folder = AR_DEPTH_FOLDER;
	readDepthFrame(gCtl.depth, depth_folder + "/depth_debug.txt");
	gCtl.depth.rgb = load_png(depth_folder + "/depth_debug.png", gCtl.depth.w, gCtl.depth.h);
#endif
	gCtl.depth.uv_quad = gU;

	Simulation sim;
	sim.init(gCtl.depth.w, gCtl.depth.h);

	glDisable(GL_DEPTH_TEST);

	GLuint depthTex = 0;
	glGenTextures_(1, &depthTex);
	glBindTexture_(GL_TEXTURE_2D, depthTex);
	glTexImage2D_(GL_TEXTURE_2D, 0, GL_R32F, gCtl.depth.w, gCtl.depth.h, 0, GL_RED, GL_FLOAT, gCtl.depth.depth.data());
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint flowTex = 0;
	glGenTextures_(1, &flowTex);
	glBindTexture_(GL_TEXTURE_2D, flowTex);
	glTexImage2D_(GL_TEXTURE_2D, 0, GL_R32F, sim.getFlowMap1().w, sim.getFlowMap1().h, 0, GL_RED, GL_FLOAT, sim.getFlowMap1().flow.data());
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLuint flowTex2 = 0;
	glGenTextures_(1, &flowTex2);
	glBindTexture_(GL_TEXTURE_2D, flowTex2);
	glTexImage2D_(GL_TEXTURE_2D, 0, GL_R32F, sim.getFlowMap2().w, sim.getFlowMap2().h, 0, GL_RED, GL_FLOAT, sim.getFlowMap2().flow.data());
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// TODO floawmap2

	std::vector<CMap> colormaps;
	// loop over *.png in folder images
	for (const auto &entry : std::filesystem::directory_iterator(folder + "/cmaps"))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".png")
		{
			CMap cmap;
			if (cmap.load(entry.path().string()))
				colormaps.push_back(std::move(cmap));
			else
				std::cerr << "Failed to load colormap: " << entry.path() << std::endl;
		}
	}

	gCtl.colormapCount = (int)colormaps.size();

	// Uniform locations
	glUseProgram_(prog);
	const GLint loc_projQuad = glGetUniformLocation_(prog, "u_projQuad");
	const GLint loc_screenSize = glGetUniformLocation_(prog, "u_screenSize");
	const GLint loc_depthUVQuad = glGetUniformLocation_(prog, "u_depthUVQuad");
	const GLint loc_depthMinMm = glGetUniformLocation_(prog, "u_depthMinMm");
	const GLint loc_depthMaxMm = glGetUniformLocation_(prog, "u_depthMaxMm");
	const GLint loc_gamma = glGetUniformLocation_(prog, "u_gamma");
	const GLint loc_depthSampler = glGetUniformLocation_(prog, "u_depthTex");
	const GLint loc_flowSampler = glGetUniformLocation_(prog, "u_flowTex");
	const GLint loc_flowSampler2 = glGetUniformLocation_(prog, "u_flowTex2");
	GLint loc_lutSampler;
	GLint u_time = -1;
	GLint u_tileScale = -1;

	loc_lutSampler = glGetUniformLocation_(prog, "u_colormapTex");

	// Bind samplers once
	glUniform1i_(loc_depthSampler, 0);
	glUniform1i_(loc_lutSampler, 1);
	glUniform1i_(loc_flowSampler, 2);
	glUniform1i_(loc_flowSampler2, 3);

	double tPrev = glfwGetTime();

	while (!glfwWindowShouldClose(win))
	{
		double tNow = glfwGetTime();
		float dt = (float)(tNow - tPrev);
		tPrev = tNow;

		// update window size (in case of resize)
		glfwGetFramebufferSize(win, &winW, &winH);
		glViewport(0, 0, winW, winH);

		// Optional gamepad controls (always safe; if no pad, no effect)
		updateGamepad(gCtl, dt);

		if (gCtl.makeItRain)
		{
			sim.randomRain();
			gCtl.makeItRain = false;
		}

		// todo
		if (gCtl.megaRain)
		{
			sim.mega1(); // random center
			gCtl.megaRain = false;
		}

		if (gCtl.spawnCreature)
		{
			sim.spawnGoat(gCtl.depth); // random pos and params
			gCtl.spawnCreature = false;
		}

		if (gCtl.clearMess)
		{
			sim.clear();
			gCtl.clearMess = false;
		}

		if (gCtl.nextBiome != 0)
		{
			if (gCtl.nextBiome > 0)
				sim.nextBiome();
			else
				sim.prevBiome();

			gCtl.nextBiome = 0;
		}

		// Update synthetic depth
		if (!gCtl.freezeDepth)
		{
#ifdef SANDBOX_WITH_REALSENSE
			realsense.grab(gCtl.depth);
			gCtl.depth.blur();
			gCtl.depth.blur();
#else
			// generateDepthFrame(gCtl.depth, tNow);
			// gCtl.depth.blur();
			// gCtl.depth.blur();
			// readDepthFrame(gCtl.depth, "depth_debug.txt");
#endif
			gCtl.depth.uv_quad = gU;

			// uint16_t min = 10000;
			// uint16_t max = 0;
			// for (auto d : gCtl.depth.depth)
			// {
			//   if (d > 0)
			//     min = std::min(d, min);
			//   max = std::max(d, max);
			// }
			// std::cout << min << " " << max << std::endl;

			glActiveTexture_(GL_TEXTURE0);
			glBindTexture_(GL_TEXTURE_2D, depthTex);
			glTexSubImage2D_(GL_TEXTURE_2D, 0, 0, 0, gCtl.depth.w, gCtl.depth.h, GL_RED, GL_FLOAT, gCtl.depth.depth.data());
		}

		sim.step(gCtl.depth, dt);

		glActiveTexture_(GL_TEXTURE2);
		glBindTexture_(GL_TEXTURE_2D, flowTex);
		glTexSubImage2D_(GL_TEXTURE_2D, 0, 0, 0, sim.getFlowMap1().w, sim.getFlowMap1().h, GL_RED, GL_FLOAT, sim.getFlowMap1().flow.data());

		glActiveTexture_(GL_TEXTURE3);
		glBindTexture_(GL_TEXTURE_2D, flowTex2);
		glTexSubImage2D_(GL_TEXTURE_2D, 0, 0, 0, sim.getFlowMap2().w, sim.getFlowMap2().h, GL_RED, GL_FLOAT, sim.getFlowMap2().flow.data());

		// todo update flowmap2

		// Clear to black
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram_(prog);

		// upload uniforms
		// proj quad: 4 vec2 => 8 floats
		float P8[8] = {gP.v[0].x, gP.v[0].y, gP.v[1].x, gP.v[1].y,
					   gP.v[2].x, gP.v[2].y, gP.v[3].x, gP.v[3].y};
		float U8[8] = {gU.v[0].x, gU.v[0].y, gU.v[1].x, gU.v[1].y,
					   gU.v[2].x, gU.v[2].y, gU.v[3].x, gU.v[3].y};

		glUniform2fv_(loc_projQuad, 4, P8);
		glUniform2f_(loc_screenSize, (float)winW, (float)winH);
		glUniform2fv_(loc_depthUVQuad, 4, U8);

		glUniform1f_(loc_depthMinMm, gCtl.depth.depthMinMm);
		glUniform1f_(loc_depthMaxMm, gCtl.depth.depthMaxMm);
		glUniform1f_(loc_gamma, gCtl.gamma);

		// bind textures
		glActiveTexture_(GL_TEXTURE0);
		glBindTexture_(GL_TEXTURE_2D, depthTex);
		glActiveTexture_(GL_TEXTURE2);
		glBindTexture_(GL_TEXTURE_2D, flowTex);
		glActiveTexture_(GL_TEXTURE3);
		glBindTexture_(GL_TEXTURE_2D, flowTex2);

		glActiveTexture_(GL_TEXTURE1);
		if (gCtl.useCMap)
			glBindTexture_(GL_TEXTURE_1D, colormaps[gCtl.colormapIndex].lutTex);
		else
			glBindTexture_(GL_TEXTURE_1D, sim.texture());

		// draw the warped quad
		glBindVertexArray_(vao);
		glDrawArrays_(GL_TRIANGLE_FAN, 0, 4);

		std::vector<OverlaySprite> sprites;

		auto detectedTags = tagDetector.detect(gCtl.depth);
		for (const auto &t : detectedTags)
		{
			auto [u, v] = gCtl.depth.inverse_warp_uv(t.uv.x, t.uv.y);

			if (t.id == 10 && t.decision_margin > 30.0f)
			{
				sim.mega2(tNow, u, v);
			}
			else if (t.id == 8 && t.decision_margin > 30.0f)
			{
				sim.mega1(tNow, u, v);
			}
			else if (t.id == 9 && t.decision_margin > 30.0f)
			{
				sim.rain(tNow, u, v);

				// sprites.push_back({
				// 	u, v,
				// 	1000.0f, // size
				// 	0.0f,
				// 	0.0f,
				// 	0.7f, 0.7f, 0.7f, // gray
				// 	0.4f              // alpha
				// });
			}
			else if (t.id == 7 && t.decision_margin > 30.0f)
			{
				const Vec2 dir = gCtl.depth.inverse_warp_dir(u, v, t.corners_px);
				sim.spawnGoat(gCtl.depth, tNow, u, v, dir.x, dir.y);
			}
			else if (t.id == 4 && t.decision_margin > 30.0f)
			{
				const Vec2 dir = gCtl.depth.inverse_warp_dir(u, v, t.corners_px);
				sim.spawnPig(gCtl.depth, tNow, u, v, dir.x, dir.y);
			}

			std::cout << "id=" << t.id
					  << " margin=" << t.decision_margin
					  << " uv=(" << t.uv.x << "," << t.uv.y << ")\n";
		}
		std::cout << std::endl;

		render_particles(sim.getRain(), sim.rainColor(), sim.rainSize(), sprites);
		render_particles(sim.getMega1(), sim.mega1Color(), sim.mega1Size(), sprites);
		render_particles(sim.getMega2(), sim.mega2Color(), sim.mega2Size(), sprites);

		for (const auto &c : sim.getCreatures())
		{
			const int frame = int(std::floor(tNow * CREATURE_ANIM_FPS)) % CREATURE_SHEET_COLS;
			const int row = static_cast<int>(c->state); // for now always use walk row; panic/dead rows can be added later

			const float u0 = float(frame) / float(CREATURE_SHEET_COLS);
			const float v0 = float(row) / float(CREATURE_SHEET_ROWS);
			const float u1 = float(frame + 1) / float(CREATURE_SHEET_COLS);
			const float v1 = float(row + 1) / float(CREATURE_SHEET_ROWS);

			sprites.push_back(
				{c->u, c->v, 30.0f, c->angle, c->flip_x,
				 1.0f, 1.0f, 1.0f, 0.95f,
				 1.0f,
				 u0, v0, u1, v1,
				 c->textureIndex()});
		}

		overlay.draw(overlayProgram, winW, winH, P8, sprites);

		glfwSwapBuffers(win);
		glfwPollEvents();

		// auto err = glGetError();
		// if (err != GL_NO_ERROR)
		//   std::cout << "GL error: " << err << "\n";
	}

	glDeleteProgram_(prog);
	glfwTerminate();
	return 0;
}

/*
Key cheat sheet:

Calibration:
  C : toggle projector-quad calibration (edit P)
  U : toggle depth-UV quad calibration (edit U)
  1 2 3 4 : select corner (TL, TR, BR, BL)
  Arrow keys : move selected corner
  Shift + arrows : fine move
  Ctrl  + arrows : coarse move

Visual / interaction:
  M / N : next / previous colormap
  [ / ] : decrease / increase depthMinMm
  ; / ' : decrease / increase depthMaxMm
  Space : freeze/unfreeze depth
  R : reset parameters
  G : cycle a few gammas

Gamepad (if present, GLFW mapping):
  A: next colormap
  Y: reset
  Left stick Y: gamma
  LB/RB: cycle colormap backward/forward
*/
