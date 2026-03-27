#include "overlay.hpp"

static const char *kOverlayVert = R"GLSL(
#version 330 core

layout(location=0) in vec2 aPos;        // [-1,1] quad vertex positions
layout(location=1) in vec2 iST;         // sandbox coords in [0,1]^2
layout(location=2) in float iRadiusPx;  // sprite radius in pixels
layout(location=3) in float iAngleRad;
layout(location=4) in float iFlipX;  // sprite flip in x direction
layout(location=5) in vec4 iColor;
layout(location=6) in float iKind;      // 0 = soft circle, 1 = textured sprite
layout(location=7) in vec4 iUVRect;     // uv0.xy, uv1.xy in [0,1]
layout(location=8) in float iTexIndex;    // texture index

uniform vec2 u_screenSize;    // pixels
uniform vec2 u_projQuad[4];   // pixels: 0 TL, 1 TR, 2 BR, 3 BL

out vec2 vLocal;
out vec2 vUV;
out vec4 vColor;
out float vKind;
out float vTexIndex;

vec2 warpScreenPx(vec2 st){
    vec2 top = mix(u_projQuad[0], u_projQuad[1], st.x);
    vec2 bot = mix(u_projQuad[3], u_projQuad[2], st.x);
    return mix(top, bot, st.y);
}

vec2 pxToNDC(vec2 p){
    return vec2(
      (p.x / u_screenSize.x) * 2.0 - 1.0,
      1.0 - (p.y / u_screenSize.y) * 2.0
    );
}

void main(){
    vec2 centerPx = warpScreenPx(iST);
    // vec2 p = centerPx + aPos * iRadiusPx;
    float c = cos(iAngleRad);
    float s = sin(iAngleRad);
    vec2 aPosRot = vec2(c * aPos.x - s * aPos.y,
                        s * aPos.x + c * aPos.y);
    vec2 p = centerPx + aPosRot * iRadiusPx;

    gl_Position = vec4(pxToNDC(p), 0.0, 1.0);
    vLocal = aPos;
    vec2 unitUV = aPos * 0.5 + 0.5;
    if (iFlipX > 0.5)
        unitUV.x = 1.0 - unitUV.x;
    vUV = mix(iUVRect.xy, iUVRect.zw, unitUV);
    vColor = iColor;
    vKind = iKind;
    vTexIndex = iTexIndex;
}
)GLSL";

static const char *kOverlayFrag = R"GLSL(
#version 330 core

in vec2 vLocal;
in vec2 vUV;
in vec4 vColor;
in float vKind;
in float vTexIndex;

uniform sampler2D u_spriteTex0;
uniform sampler2D u_spriteTex1;

out vec4 FragColor;

void main(){
    if (vKind < 0.5) {
        float r = length(vLocal);
        float alpha = 1.0 - smoothstep(0.85, 1.0, r);
        FragColor = vec4(vColor.rgb, vColor.a * alpha);
        return;
    }

    vec4 texel = (vTexIndex > 0.5f)
    ? texture(u_spriteTex1, vUV)
    : texture(u_spriteTex0, vUV);
    FragColor = vec4(vColor.rgb * texel.rgb, vColor.a * texel.a);
}
)GLSL";

OverlayRenderer::OverlayRenderer()
{
	// Two triangles (6 verts), aPos in [-1,1]
	const float quadVerts[] = {-1.f, -1.f, 1.f, -1.f, 1.f, 1.f,
							   -1.f, -1.f, 1.f, 1.f, -1.f, 1.f};

	glGenVertexArrays_(1, &vao);
	glBindVertexArray_(vao);

	// Quad vertex buffer
	glGenBuffers_(1, &quadVBO);
	glBindBuffer_(GL_ARRAY_BUFFER, quadVBO);
	glBufferData_(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

	// location 0: aPos
	glEnableVertexAttribArray_(0);
	glVertexAttribPointer_(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
						   (void *)0);

	// Instance buffer (streamed every frame)
	glGenBuffers_(1, &instVBO);
	glBindBuffer_(GL_ARRAY_BUFFER, instVBO);
	glBufferData_(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);

	// Each instance:
	// st_x, st_y, radius_px, angle_rad, r, g, b, a, kind, flip_x,
	// uv0_x, uv0_y, uv1_x, uv1_y, texIndex  (15 floats)
	const GLsizei stride = 15 * sizeof(float);

	// location 1: iST (vec2)
	glEnableVertexAttribArray_(1);
	glVertexAttribPointer_(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(0));
	glVertexAttribDivisor_(1, 1);

	// location 2: iRadiusPx (float)
	glEnableVertexAttribArray_(2);
	glVertexAttribPointer_(2, 1, GL_FLOAT, GL_FALSE, stride,
						   (void *)(2 * sizeof(float)));
	glVertexAttribDivisor_(2, 1);

	// location 3: iAngleRad (float)
	glEnableVertexAttribArray_(3);
	glVertexAttribPointer_(3, 1, GL_FLOAT, GL_FALSE, stride,
						   (void *)(3 * sizeof(float)));
	glVertexAttribDivisor_(3, 1);

	// location 4: iFlipX (float)
	glEnableVertexAttribArray_(4);
	glVertexAttribPointer_(4, 1, GL_FLOAT, GL_FALSE, stride,
						   (void *)(4 * sizeof(float)));
	glVertexAttribDivisor_(4, 1);

	// location 4: iColor (vec4)
	glEnableVertexAttribArray_(5);
	glVertexAttribPointer_(5, 4, GL_FLOAT, GL_FALSE, stride,
						   (void *)(5 * sizeof(float)));
	glVertexAttribDivisor_(5, 1);

	// location 5: iKind (float)
	glEnableVertexAttribArray_(6);
	glVertexAttribPointer_(6, 1, GL_FLOAT, GL_FALSE, stride,
						   (void *)(9 * sizeof(float)));
	glVertexAttribDivisor_(6, 1);

	// location 6: iUVRect (vec4)
	glEnableVertexAttribArray_(7);
	glVertexAttribPointer_(7, 4, GL_FLOAT, GL_FALSE, stride,
						   (void *)(10 * sizeof(float)));
	glVertexAttribDivisor_(7, 1);

	// location 7: iTexIndex (float)
	glEnableVertexAttribArray_(8);
	glVertexAttribPointer_(8, 1, GL_FLOAT, GL_FALSE, stride,
						   (void *)(14 * sizeof(float)));
	glVertexAttribDivisor_(8, 1);

	glBindVertexArray_(0);
	glBindBuffer_(GL_ARRAY_BUFFER, 0);

	const uint8_t white[4] = {255, 255, 255, 255};
	createRGBA8Texture(white, 1, 1, 0);
	createRGBA8Texture(white, 1, 1, 1);
}

void OverlayRenderer::createRGBA8Texture(const uint8_t *rgba, int w, int h, int index)
{
	GLuint tex = 0;
	glGenTextures_(1, &tex);
	glBindTexture_(GL_TEXTURE_2D, tex);
	glTexImage2D_(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
				  rgba);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture_(GL_TEXTURE_2D, 0);

	spriteTex[index] = tex;
}

void OverlayRenderer::draw(
	GLuint overlayProgram, int screenW, int screenH,
	const float projQuadPx[8], // [x0,y0, x1,y1, x2,y2, x3,y3] TL,TR,BR,BL
	const std::vector<OverlaySprite> &sprites)
{
	if (sprites.empty())
		return;

	glUseProgram_(overlayProgram);

	// uniforms
	GLint locSize = glGetUniformLocation_(overlayProgram, "u_screenSize");
	glUniform2f_(locSize, (float)screenW, (float)screenH);

	// u_projQuad is an array of vec2 => upload 4 vec2 as 8 floats
	GLint locQuad = glGetUniformLocation_(overlayProgram, "u_projQuad");
	glUniform2fv_(locQuad, 4, projQuadPx);

	GLint locSpriteTex0 = glGetUniformLocation_(overlayProgram, "u_spriteTex0");
	glUniform1i_(locSpriteTex0, 3);
	glActiveTexture_(GL_TEXTURE3);
	glBindTexture_(GL_TEXTURE_2D, spriteTex[0]);
	GLint locSpriteTex1 = glGetUniformLocation_(overlayProgram, "u_spriteTex1");
	glUniform1i_(locSpriteTex1, 4);
	glActiveTexture_(GL_TEXTURE4);
	glBindTexture_(GL_TEXTURE_2D, spriteTex[1]);

	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindVertexArray_(vao);

	// Upload instance buffer as raw floats
	glBindBuffer_(GL_ARRAY_BUFFER, instVBO);
	glBufferData_(GL_ARRAY_BUFFER,
				  (GLsizeiptr)(sprites.size() * sizeof(OverlaySprite)),
				  sprites.data(), GL_STREAM_DRAW);

	// Draw 6 vertices per quad, instanced
	glDrawArraysInstanced_(GL_TRIANGLES, 0, 6, (GLsizei)sprites.size());

	glBindVertexArray_(0);
	glBindBuffer_(GL_ARRAY_BUFFER, 0);
}

// Returns 0 on failure.
GLuint OverlayRenderer::createProgram()
{
	GLuint vs = compileShader(GL_VERTEX_SHADER, kOverlayVert);
	if (!vs)
		return 0;
	GLuint fs = compileShader(GL_FRAGMENT_SHADER, kOverlayFrag);
	if (!fs)
	{
		glDeleteShader_(vs);
		return 0;
	}

	GLuint prog = linkProgram(vs, fs);

	// shaders can be deleted after linking
	glDeleteShader_(vs);
	glDeleteShader_(fs);

	return prog;
}
