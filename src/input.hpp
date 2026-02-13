#pragma once

#include "utils.hpp"

// ----------------------------- Input Helpers --------------------------------
static float stepScale(int mods, float base) {
  if (mods & GLFW_MOD_SHIFT)
    return base * 0.2f; // fine
  if (mods & GLFW_MOD_CONTROL)
    return base * 5.0f; // coarse
  return base;
}

static void updateGamepad(Controls &c, float dt) {
  c.gamepadPresent = glfwJoystickPresent(GLFW_JOYSTICK_1) &&
                     glfwJoystickIsGamepad(GLFW_JOYSTICK_1);
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

  // gamma with left Y
  c.gamma += (-ly) * (0.8f * dt);
  c.gamma = std::clamp(c.gamma, 0.2f, 3.0f);

  // buttons
  bool A = (s.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS);
  bool B = (s.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS);
  bool X = (s.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS);
  bool Y = (s.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS);
  bool LB = (s.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS);
  bool RB = (s.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS);

  if (c.bEdge.pressed(B))
    c.makeItRain = true;

  if (c.aEdge.pressed(A))
    c.colormapIndex = (c.colormapIndex + 1) % std::max(1, c.colormapCount);
  if (c.yEdge.pressed(Y))
    c.reset();
  if (c.lbEdge.pressed(LB))
    c.colormapIndex =
        (c.colormapIndex - 1 + c.colormapCount) % std::max(1, c.colormapCount);
  if (c.rbEdge.pressed(RB))
    c.colormapIndex = (c.colormapIndex + 1) % std::max(1, c.colormapCount);
}

static void moveSelectedCorner(Quad &Q, int key, float step) {
  switch (key) {
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
                        int mods) {
  if (action != GLFW_PRESS && action != GLFW_REPEAT)
    return;

  // mode toggles (press only)
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_C) {
      gMode = (gMode == PROJ) ? NONE : PROJ;
      return;
    }
    if (key == GLFW_KEY_U) {
      gMode = (gMode == UV) ? NONE : UV;
      return;
    }

    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4) {
      gSelCorner = key - GLFW_KEY_1; // 0..3
      return;
    }

    // colormap cycle
    if (key == GLFW_KEY_M) {
      gCtl.colormapIndex =
          (gCtl.colormapIndex + 1) % std::max(1, gCtl.colormapCount);
      return;
    }
    if (key == GLFW_KEY_N) {
      gCtl.colormapIndex = (gCtl.colormapIndex - 1 + gCtl.colormapCount) %
                           std::max(1, gCtl.colormapCount);
      return;
    }

    if (key == GLFW_KEY_SPACE) {
      gCtl.freezeDepth = !gCtl.freezeDepth;
      return;
    }
    if (key == GLFW_KEY_R) {
      gCtl.reset();
      return;
    }

    if (key == GLFW_KEY_T) {
      gCtl.makeItRain = true;
      return;
    }
  }

  // calibration movement (press+repeat)
  if (gMode != NONE) {
    float step = 5.0f;
    if (mods & GLFW_MOD_SHIFT)
      step = 1.0f;
    if (mods & GLFW_MOD_CONTROL)
      step = 25.0f;

    if (gMode == PROJ) {
      moveSelectedCorner(gP, key, step);
    } else if (gMode == UV) {
      // UV uses normalized coordinates: scale step down
      float uvStep = step / 2000.0f; // tweak; feels ok for repeat
      moveSelectedCorner(gU, key, uvStep);
      gU.v[gSelCorner] = clamp01(gU.v[gSelCorner]);
    }
    return;
  }

  // Depth min/max "chaos knobs"
  if (key == GLFW_KEY_LEFT_BRACKET) { // '['
    gCtl.depth.depthMinMm -= stepScale(mods, 10.0f);
    return;
  }
  if (key == GLFW_KEY_RIGHT_BRACKET) { // ']'
    gCtl.depth.depthMinMm += stepScale(mods, 10.0f);
    return;
  }
  if (key == GLFW_KEY_SEMICOLON) { // ';'
    gCtl.depth.depthMaxMm -= stepScale(mods, 10.0f);
    return;
  }
  if (key == GLFW_KEY_APOSTROPHE) { // '''
    gCtl.depth.depthMaxMm += stepScale(mods, 10.0f);
    return;
  }
  if (gCtl.depth.depthMaxMm < gCtl.depth.depthMinMm + 50.0f)
    gCtl.depth.depthMaxMm = gCtl.depth.depthMinMm + 50.0f;
  // Gamma
  if (key == GLFW_KEY_G) {
    // tap G cycles common gammas; hold with repeat will still just cycle
    // quickly
    if (action == GLFW_PRESS) {
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
}
