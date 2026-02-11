#include "overlay.hpp"

static const char *kOverlayVert = R"GLSL(
#version 330 core

layout(location=0) in vec2 aPos;        // [-1,1] quad vertex positions
layout(location=1) in vec2 iST;         // sandbox coords in [0,1]^2
layout(location=2) in float iRadiusPx;  // sprite radius in pixels
layout(location=3) in vec4 iColor;

uniform vec2 u_screenSize;    // pixels
uniform vec2 u_projQuad[4];   // pixels: 0 TL, 1 TR, 2 BR, 3 BL

out vec2 vLocal;
out vec4 vColor;

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
    vec2 p = centerPx + aPos * iRadiusPx;

    gl_Position = vec4(pxToNDC(p), 0.0, 1.0);
    vLocal = aPos;
    vColor = iColor;
}
)GLSL";

static const char *kOverlayFrag = R"GLSL(
#version 330 core

in vec2 vLocal;
in vec4 vColor;

out vec4 FragColor;

void main(){
    float r = length(vLocal);
    float alpha = 1.0 - smoothstep(0.85, 1.0, r);
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
)GLSL";

OverlayRenderer overlayInit() {
  OverlayRenderer R;

  // Two triangles (6 verts), aPos in [-1,1]
  const float quadVerts[] = {-1.f, -1.f, 1.f, -1.f, 1.f,  1.f,
                             -1.f, -1.f, 1.f, 1.f,  -1.f, 1.f};

  glGenVertexArrays(1, &R.vao);
  glBindVertexArray(R.vao);

  // Quad vertex buffer
  glGenBuffers_(1, &R.quadVBO);
  glBindBuffer_(GL_ARRAY_BUFFER, R.quadVBO);
  glBufferData_(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

  // location 0: aPos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

  // Instance buffer (streamed every frame)
  glGenBuffers_(1, &R.instVBO);
  glBindBuffer_(GL_ARRAY_BUFFER, R.instVBO);
  glBufferData_(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);

  // Each instance: st_x, st_y, radius_px, r, g, b, a  (7 floats)
  const GLsizei stride = 7 * sizeof(float);

  // location 1: iST (vec2)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(0));
  glVertexAttribDivisor(1, 1);

  // location 2: iRadiusPx (float)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride,
                        (void *)(2 * sizeof(float)));
  glVertexAttribDivisor(2, 1);

  // location 3: iColor (vec4)
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride,
                        (void *)(3 * sizeof(float)));
  glVertexAttribDivisor(3, 1);

  glBindVertexArray(0);
  glBindBuffer_(GL_ARRAY_BUFFER, 0);

  return R;
}

void overlayDraw(
    const OverlayRenderer &R, GLuint overlayProgram, int screenW, int screenH,
    const float projQuadPx[8], // [x0,y0, x1,y1, x2,y2, x3,y3] TL,TR,BR,BL
    const std::vector<OverlaySprite> &sprites) {
  if (sprites.empty())
    return;

  glUseProgram(overlayProgram);

  // uniforms
  GLint locSize = glGetUniformLocation(overlayProgram, "u_screenSize");
  glUniform2f(locSize, (float)screenW, (float)screenH);

  // u_projQuad is an array of vec2 => upload 4 vec2 as 8 floats
  GLint locQuad = glGetUniformLocation(overlayProgram, "u_projQuad");
  glUniform2fv(locQuad, 4, projQuadPx);

  // Enable alpha blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(R.vao);

  // Upload instance buffer as raw floats
  glBindBuffer_(GL_ARRAY_BUFFER, R.instVBO);
  glBufferData_(GL_ARRAY_BUFFER,
                (GLsizeiptr)(sprites.size() * sizeof(OverlaySprite)),
                sprites.data(), GL_STREAM_DRAW);

  // Draw 6 vertices per quad, instanced
  glDrawArraysInstanced_(GL_TRIANGLES, 0, 6, (GLsizei)sprites.size());

  glBindVertexArray(0);
  glBindBuffer_(GL_ARRAY_BUFFER, 0);
}

// Returns 0 on failure.
GLuint createOverlayProgram() {
  GLuint vs = compileShader(GL_VERTEX_SHADER, kOverlayVert);
  if (!vs)
    return 0;
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, kOverlayFrag);
  if (!fs) {
    glDeleteShader(vs);
    return 0;
  }

  GLuint prog = linkProgram(vs, fs);

  // shaders can be deleted after linking
  glDeleteShader(vs);
  glDeleteShader(fs);

  return prog;
}
