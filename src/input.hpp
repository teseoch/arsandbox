#pragma once

#include "utils.hpp"
#include "audio.hpp"

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

enum class InputActionType
{
	Rain,
	Mega1,
	Mega2,
	Clear,
	NextBiome,
	PrevBiome,
	SpawnGoat,
	SpawnPig,
	SpawnFish,
	SpawnEagle,
	SpawnVulture,
	SpawnFrog
};

struct Controls
{
	Depth depth;

	float gamma = 1.0f;

	std::vector<InputActionType> actions;

	bool useCMap = false;
	int colormapIndex = 0;
	int colormapCount = 0;

	bool freezeDepth = false;

	// Gamepad edges
	ButtonEdge aEdge, bEdge, xEdge, yEdge, lbEdge, rbEdge, dpadUpEdge, dpadLeftEdge, dpadRightEdge, dpadDownEdge;
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

	// buttons
	bool A = (s.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS || s.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_REPEAT);
	bool B = (s.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS || s.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_REPEAT);
	bool X = (s.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS);
	bool Y = (s.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS);

	bool LB = (s.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS);
	bool RB = (s.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS);

	bool DUp = (s.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS);
	bool DLeft = (s.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS);
	bool DRight = (s.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS);
	bool DDown = (s.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS);

	if (c.bEdge.pressed(B))
		c.actions.push_back(InputActionType::Rain);
	if (c.xEdge.pressed(X))
		c.actions.push_back(InputActionType::Mega1);
	if (c.aEdge.pressed(A))
		c.actions.push_back(InputActionType::Mega2);

	if (c.yEdge.pressed(Y))
		c.actions.push_back(InputActionType::Clear);

	if (c.dpadUpEdge.pressed(DUp))
		c.actions.push_back(InputActionType::SpawnGoat);
	if (c.dpadLeftEdge.pressed(DLeft))
		c.actions.push_back(InputActionType::SpawnPig);
	if (c.dpadRightEdge.pressed(DRight))
		c.actions.push_back(InputActionType::SpawnFish);
	if (c.dpadDownEdge.pressed(DDown))
		c.actions.push_back(InputActionType::SpawnEagle);

	if (c.lbEdge.pressed(LT))
	{
		c.actions.push_back(InputActionType::SpawnVulture);
		c.useCMap = true;
	}
	if (c.rbEdge.pressed(RT))
	{
		c.actions.push_back(InputActionType::SpawnFrog);
		c.useCMap = true;
	}

	if (c.rbEdge.pressed(LB))
	{
		c.actions.push_back(InputActionType::PrevBiome);
		c.useCMap = false;
	}
	if (c.lbEdge.pressed(RB))
	{
		c.actions.push_back(InputActionType::NextBiome);
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
		gCtl.actions.push_back(InputActionType::Rain);
		return;
	}

	if (key == GLFW_KEY_5)
	{
		gCtl.actions.push_back(InputActionType::SpawnGoat);
		return;
	}
	if (key == GLFW_KEY_6)
	{
		gCtl.actions.push_back(InputActionType::SpawnPig);
		return;
	}
	if (key == GLFW_KEY_7)
	{
		gCtl.actions.push_back(InputActionType::SpawnFish);
		return;
	}
	if (key == GLFW_KEY_8)
	{
		gCtl.actions.push_back(InputActionType::SpawnEagle);
		return;
	}
	if (key == GLFW_KEY_9)
	{
		gCtl.actions.push_back(InputActionType::SpawnVulture);
		return;
	}
	if (key == GLFW_KEY_0)
	{
		gCtl.actions.push_back(InputActionType::SpawnFrog);
		return;
	}

	// mode toggles (press only)
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_K)
		{
			gCtl.actions.push_back(InputActionType::Mega1);
			return;
		}
		if (key == GLFW_KEY_L)
		{
			gCtl.actions.push_back(InputActionType::Mega2);
			return;
		}

		if (key == GLFW_KEY_Y)
		{
			gCtl.actions.push_back(InputActionType::Clear);
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

		// biome cycle
		if (key == GLFW_KEY_N)
		{
			gCtl.actions.push_back(InputActionType::PrevBiome);
			gCtl.useCMap = false;
			return;
		}
		if (key == GLFW_KEY_M)
		{
			gCtl.actions.push_back(InputActionType::NextBiome);
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
