#pragma once

#include "utils.hpp"

#include <cstdint>
#include <vector>

struct OverlaySprite
{
	float st_x;             // [0,1]
	float st_y;             // [0,1]
	float radius_px;        // pixels
	float angle_rad = 0.0f; // sprite rotation in radians
	float flip_x = 0.0f;    // sprite flip in x direction
	float r, g, b, a;       // [0,1]

	// Optional textured-sprite support. Existing aggregate initializers that only
	// provide the first 7 floats will keep using the default circle sprite mode.
	float kind = 0.0f; // 0 = soft circle, 1 = textured sprite
	float uv0_x = 0.0f;
	float uv0_y = 0.0f;
	float uv1_x = 1.0f;
	float uv1_y = 1.0f;
	float texIndex = 0;
};

class OverlayRenderer
{
public:
	GLuint vao = 0;
	GLuint quadVBO = 0;
	GLuint instVBO = 0;
	GLuint spriteTex[2][2] = {{0, 0}, {0, 0}}; // [biome][animal]

	OverlayRenderer();

	void createRGBA8Texture(const uint8_t *rgba, int w, int h, int biome, int animal);

	void draw(
		GLuint overlayProgram, int screenW, int screenH,
		const float projQuadPx[8], // [x0,y0, x1,y1, x2,y2, x3,y3] TL,TR,BR,BL
		const int biome,
		const std::vector<OverlaySprite> &sprites);

	GLuint createProgram();
};