#pragma once

#include "utils.hpp"

#include <fstream>
#include <iostream>

// ----------------------------- Controls / State -----------------------------
enum Mode
{
	NONE,
	PROJ,
	UV
};

struct ButtonEdge
{
	bool prev = false;
	bool pressed(bool now)
	{
		bool r = (now && !prev);
		prev = now;
		return r;
	}
};

struct Controls
{
	Depth depth;

	float gamma = 1.0f;

	int nextBiome = 0;
	bool useCMap = false;
	int colormapIndex = 0;
	int colormapCount = 0;

	bool freezeDepth = false;
	bool makeItRain = false;
	bool megaRain = false;
	bool clearMess = false;
	bool spawnCreature = false;

	// Gamepad edges
	ButtonEdge aEdge, bEdge, xEdge, yEdge, lbEdge, rbEdge;
	bool gamepadPresent = false;

	void reset()
	{
#ifndef SANDBOX_WITH_REALSENSE
		depth.w = 320;
		depth.h = 240;
#endif
		gamma = 1.0f;
		colormapIndex = 0;
		freezeDepth = false;
	}
};

// ----------------------------- Input Helpers --------------------------------
static float stepScale(int mods, float base)
{
	if (mods & GLFW_MOD_SHIFT)
		return base * 0.2f; // fine
	if (mods & GLFW_MOD_CONTROL)
		return base * 5.0f; // coarse
	return base;
}

static inline Vec2 clamp01(Vec2 p)
{
	p.x = std::max(0.0f, std::min(1.0f, p.x));
	p.y = std::max(0.0f, std::min(1.0f, p.y));
	return p;
}

static Mode gMode = NONE;
static int gSelCorner = 0;

static Controls gCtl;

// Projector quad in *window pixel coords* (you warp this to match box corners)
static Quad gP;
// Depth UV quad in *normalized* [0,1] coords (you warp this to match depth ROI)
static Quad gU;

static void updateGamepad(Controls &c, float dt)
{
	c.gamepadPresent = glfwJoystickPresent(GLFW_JOYSTICK_1) && glfwJoystickIsGamepad(GLFW_JOYSTICK_1);
	if (!c.gamepadPresent)
		return;

	GLFWgamepadstate s;
	if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &s))
		return;

	auto deadzone = [&](float x) { return (std::fabs(x) < 0.12f) ? 0.0f : x; };

	float ly = deadzone(s.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]); // -1 up, +1 down
	float ry = deadzone(s.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
	float rt =
		s.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]; // -1 released -> +1 pressed
	float lt = s.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
	float RT = std::clamp((rt + 1.0f) * 0.5f, 0.0f, 1.0f);
	float LT = std::clamp((lt + 1.0f) * 0.5f, 0.0f, 1.0f);

	// // gamma with left Y
	// c.gamma += (-ly) * (0.8f * dt);
	// c.gamma = std::clamp(c.gamma, 0.2f, 3.0f);

	// buttons
	bool A = (s.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS || s.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_REPEAT);
	bool B = (s.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS || s.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_REPEAT);
	bool X = (s.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS);
	bool Y = (s.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS);
	bool LB = (s.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS);
	bool RB = (s.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS);

	if (c.bEdge.pressed(B))
		c.makeItRain = true;
	if (c.aEdge.pressed(X))
		c.megaRain = true;

	if (c.yEdge.pressed(Y))
		c.clearMess = true;

	if (c.aEdge.pressed(A))
		c.spawnCreature = true;
	// spawn creature
	// if (c.yEdge.pressed(Y))
	//   clear rain and creatures
	// 	X = mega rain burst

	if (c.lbEdge.pressed(LT))
	{
		c.colormapIndex = (c.colormapIndex - 1 + c.colormapCount) % std::max(1, c.colormapCount);
		c.useCMap = true;
	}
	if (c.rbEdge.pressed(RT))
	{
		c.colormapIndex = (c.colormapIndex + 1) % std::max(1, c.colormapCount);
		c.useCMap = true;
	}

	if (c.rbEdge.pressed(LB))
	{
		c.nextBiome = -1;
		c.useCMap = false;
	}
	if (c.lbEdge.pressed(RB))
	{
		c.nextBiome = 1;
		c.useCMap = false;
	}
}

static void moveSelectedCorner(Quad &Q, int key, float step)
{
	switch (key)
	{
	case GLFW_KEY_LEFT:
		Q.v[gSelCorner].x -= step;
		break;
	case GLFW_KEY_RIGHT:
		Q.v[gSelCorner].x += step;
		break;
	case GLFW_KEY_UP:
		Q.v[gSelCorner].y -= step;
		break;
	case GLFW_KEY_DOWN:
		Q.v[gSelCorner].y += step;
		break;
	default:
		break;
	}
}

// Keyboard: calibration + knobs
static void keyCallback(GLFWwindow *w, int key, int scancode, int action,
						int mods)
{

	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;

	if (key == GLFW_KEY_T)
	{
		gCtl.makeItRain = true;
		return;
	}

	// mode toggles (press only)
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_K)
		{
			gCtl.megaRain = true;
			return;
		}
		if (key == GLFW_KEY_Y)
		{
			gCtl.clearMess = true;
			return;
		}
		if (key == GLFW_KEY_L)
		{
			gCtl.spawnCreature = true;
			return;
		}

		if (key == GLFW_KEY_C)
		{
			gMode = (gMode == PROJ) ? NONE : PROJ;
			return;
		}
		if (key == GLFW_KEY_U)
		{
			gMode = (gMode == UV) ? NONE : UV;
			return;
		}

		if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4)
		{
			gSelCorner = key - GLFW_KEY_1; // 0..3
			return;
		}

		// colormap cycle
		if (key == GLFW_KEY_N)
		{
			gCtl.nextBiome = -1;
			gCtl.useCMap = false;
			return;
		}
		if (key == GLFW_KEY_M)
		{
			gCtl.nextBiome = -1;
			gCtl.useCMap = false;
			return;
		}

		// colormap cycle
		if (key == GLFW_KEY_I)
		{
			gCtl.colormapIndex =
				(gCtl.colormapIndex + 1) % std::max(1, gCtl.colormapCount);
			gCtl.useCMap = true;
			return;
		}
		if (key == GLFW_KEY_O)
		{
			gCtl.colormapIndex = (gCtl.colormapIndex - 1 + gCtl.colormapCount) % std::max(1, gCtl.colormapCount);
			gCtl.useCMap = true;
			return;
		}

		if (key == GLFW_KEY_SPACE)
		{
			gCtl.freezeDepth = !gCtl.freezeDepth;
			return;
		}
		if (key == GLFW_KEY_R)
		{
			gCtl.reset();
			return;
		}
	}

	// calibration movement (press+repeat)
	if (gMode != NONE)
	{
		float step = 5.0f;
		if (mods & GLFW_MOD_SHIFT)
			step = 1.0f;
		if (mods & GLFW_MOD_CONTROL)
			step = 25.0f;

		if (gMode == PROJ)
		{
			moveSelectedCorner(gP, key, step);
		}
		else if (gMode == UV)
		{
			// UV uses normalized coordinates: scale step down
			float uvStep = step / 2000.0f; // tweak; feels ok for repeat
			moveSelectedCorner(gU, key, uvStep);
			gU.v[gSelCorner] = clamp01(gU.v[gSelCorner]);
		}

		std::ofstream log("calib.txt");
		for (int i = 0; i < 4; i++)
			log << gP.v[i].x << " " << gP.v[i].y << "\n";
		for (int i = 0; i < 4; i++)
			log << gU.v[i].x << " " << gU.v[i].y << "\n";
		return;
	}

	// Depth min/max "chaos knobs"
	if (key == GLFW_KEY_LEFT_BRACKET)
	{ // '['
		gCtl.depth.depthMinMm -= stepScale(mods, 1.0f);
		std::cout << gCtl.depth.depthMinMm << std::endl;
		std::cout << gCtl.depth.depthMaxMm << std::endl;
		return;
	}
	if (key == GLFW_KEY_RIGHT_BRACKET)
	{ // ']'
		gCtl.depth.depthMinMm += stepScale(mods, 1.0f);
		std::cout << gCtl.depth.depthMinMm << std::endl;
		std::cout << gCtl.depth.depthMaxMm << std::endl;
		return;
	}
	if (key == GLFW_KEY_SEMICOLON)
	{ // ';'
		gCtl.depth.depthMaxMm -= stepScale(mods, 1.0f);
		std::cout << gCtl.depth.depthMinMm << std::endl;
		std::cout << gCtl.depth.depthMaxMm << std::endl;
		return;
	}
	if (key == GLFW_KEY_APOSTROPHE)
	{ // '''
		gCtl.depth.depthMaxMm += stepScale(mods, 1.0f);
		std::cout << gCtl.depth.depthMinMm << std::endl;
		std::cout << gCtl.depth.depthMaxMm << std::endl;
		return;
	}
	// if (gCtl.depth.depthMaxMm < gCtl.depth.depthMinMm + 50.0f)
	//   gCtl.depth.depthMaxMm = gCtl.depth.depthMinMm + 50.0f;
	// Gamma
	if (key == GLFW_KEY_G)
	{
		// tap G cycles common gammas; hold with repeat will still just cycle
		// quickly
		if (action == GLFW_PRESS)
		{
			if (std::fabs(gCtl.gamma - 1.0f) < 1e-3f)
				gCtl.gamma = 0.8f;
			else if (std::fabs(gCtl.gamma - 0.8f) < 1e-3f)
				gCtl.gamma = 1.2f;
			else if (std::fabs(gCtl.gamma - 1.2f) < 1e-3f)
				gCtl.gamma = 1.6f;
			else
				gCtl.gamma = 1.0f;
		}
		return;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		gCtl.depth.save_png("depth_debug.png");
		std::cout << "Saved depth_debug.png\n";
	}

	if (key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(w, GLFW_TRUE);
		return;
	}
}
