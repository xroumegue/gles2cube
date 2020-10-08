/*
 * GLES2 Cube
 * Demo of a simple rotating cube, with many native windowing support
 */

/*
 * This program is distributer under the 2-clause BSD license.
 * See at the end of this file for details.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <math.h>

#include "native_gfx.h"
#include "egl_helper.h"
#include "gles_helper.h"
#include "esUtil.h"

#include "gles2cube.h"

static const GLfloat
cube_vertices_g[] = {
   -0.5f, -0.5f, -0.5f,
   -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f, -0.5f, -0.5f,
   -0.5f,  0.5f, -0.5f,
   -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
   -0.5f, -0.5f, -0.5f,
   -0.5f,  0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
   -0.5f, -0.5f,  0.5f,
   -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
   -0.5f, -0.5f, -0.5f,
   -0.5f, -0.5f,  0.5f,
   -0.5f,  0.5f,  0.5f,
   -0.5f,  0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
    0.5f, -0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    0.5f,  0.5f, -0.5f,
};

static const GLfloat
cube_tex_coords_g[] = {
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
};

static const int cube_num_indices_g = 36;
static const GLuint
cube_indices_g[] = {
    0, 2, 1,
    0, 3, 2,

    4, 5, 6,
    4, 6, 7,

    8, 9, 10,
    8, 10, 11,

    12, 15, 14,
    12, 14, 13,

    16, 17, 18,
    16, 18, 19,

    20, 23, 22,
    20, 22, 21
};

#define TEX_WIDTH 8
#define TEX_HEIGHT 8
static const GLubyte
tex_data_g[TEX_WIDTH * TEX_HEIGHT] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
  0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const GLchar *
vertex_shader_g =
  "precision mediump float;                   \n"

  "uniform mat4 u_mat;                        \n"
  "attribute vec4 a_pos;                      \n"
  "attribute vec2 a_uv;                       \n"
  "varying vec2 v_uv;                         \n"

  "void main() {                              \n"
  "   gl_Position = u_mat * a_pos;            \n"
  "   v_uv = a_uv;                            \n"
  "}\n";

static const GLchar *
fragment_shader_g =
  "precision mediump float;                           \n"
  "uniform vec4 u_color;                              \n"
  "uniform float u_alpha;                             \n"
  "uniform sampler2D u_tex;                           \n"
  "varying vec2 v_uv;                                 \n"
  "void main() {                                      \n"

  "vec4 t = texture2D(u_tex, v_uv);                   \n"
  "       gl_FragColor = mix(u_color, t, u_alpha);    \n"
  "}\n";

static void
cleanup(context_t *ctx)
{
  XglUseProgram(0);
  XglDetachShader(ctx->gl_prog, ctx->vertex_shader);
  XglDetachShader(ctx->gl_prog, ctx->fragment_shader);
  XglDeleteShader(ctx->vertex_shader);
  XglDeleteShader(ctx->fragment_shader);
  XglDeleteProgram(ctx->gl_prog);

  egl_clean(ctx->egl);
}

static GLuint
load_shader(GLenum type,
            GLsizei count,
            const GLchar * GL_SHADER_SOURCE_CONST *shaderSrc)
{
  GLuint shader;
  GLint compiled = GL_FALSE;

  shader = XglCreateShader(type);
  if (shader == 0)
    return (0);

  XglShaderSource(shader, count, shaderSrc, NULL);
  XglCompileShader(shader);
  XglGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

  if (compiled != GL_TRUE) {
    GLint info_len = 0;

    XglGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);

    if (info_len > 1) {
      char* info_log = malloc(info_len);

      XglGetShaderInfoLog(shader, info_len, NULL, info_log);
      fprintf(stderr, "Info log from shader compilation:\n%s\n", info_log);
      free(info_log);
    }

    XglDeleteShader(shader);

    return (0);
  }

  return (shader);
}

static void
load_program(GLint v_shader_count,
             const GLchar * GL_SHADER_SOURCE_CONST *v_shader_src,
             GLint f_shader_count,
             const GLchar * GL_SHADER_SOURCE_CONST *f_shader_src,
             GLuint *vshader, GLuint *fshader, GLuint *program)
{
  GLuint vsh;
  GLuint fsh;
  GLuint prog;
  GLint  linked = GL_FALSE;

  vsh = load_shader(GL_VERTEX_SHADER, v_shader_count, v_shader_src);
  if (vsh == 0) {
    *program = 0;
    return ;
  }

  fsh = load_shader(GL_FRAGMENT_SHADER, f_shader_count, f_shader_src);
  if (fsh == 0) {
    XglDeleteShader(vsh);
    *program = 0;
    return ;
  }

  prog = XglCreateProgram();
  if (prog == 0) {
    XglDeleteShader(vsh);
    XglDeleteShader(fsh);
    *program = 0;
    return ;
  }

  XglAttachShader(prog, vsh);
  XglAttachShader(prog, fsh);
  XglLinkProgram(prog);

  XglGetProgramiv(prog, GL_LINK_STATUS, &linked);

  if (linked != GL_TRUE) {
    GLint info_len = 0;

    XglGetProgramiv(prog, GL_INFO_LOG_LENGTH, &info_len);
    if (info_len > 0) {
      char* info_log = malloc(info_len);
      if (info_log == NULL) {
        fprintf(stderr, "ERROR: malloc(): errno %i: %s\n",
                errno, strerror(errno));
        exit(EXIT_FAILURE);
      }

      XglGetProgramInfoLog(prog, info_len, NULL, info_log);
      fprintf(stderr, "Info log from program linking:\n%s\n", info_log);
      free(info_log);
    }

    XglDeleteShader(vsh);
    XglDeleteShader(fsh);
    XglDeleteProgram(prog);
    *program = 0;

    return ;
  }

  *vshader = vsh;
  *fshader = fsh;
  *program = prog;
}

static void
setup_texture(context_t *ctx)
{
  glActiveTexture(GL_TEXTURE0);

  XglGenTextures(1, &ctx->gl_tex_id);
  XglBindTexture(GL_TEXTURE_2D, ctx->gl_tex_id);

  XglTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE,
                TEX_WIDTH, TEX_HEIGHT, 0,
                GL_LUMINANCE,
                GL_UNSIGNED_BYTE, tex_data_g);

  XglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  XglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

static void
setup(context_t *ctx)
{
  ESMatrix view, pers;
  const GLchar *v_src[1];
  const GLchar *f_src[1];

  v_src[0] = vertex_shader_g;
  f_src[0] = fragment_shader_g;

  load_program(1, v_src, 1, f_src,
               &ctx->vertex_shader, &ctx->fragment_shader, &ctx->gl_prog);
  if (ctx->gl_prog == 0) {
    fprintf(stderr, "ERROR: while loading shaders and program.\n");
    exit(EXIT_FAILURE);
  }

  XglUseProgram(ctx->gl_prog);

  /* setup uniforms */
  ctx->u_mat = XglGetUniformLocation(ctx->gl_prog, "u_mat");
  ctx->u_color = XglGetUniformLocation(ctx->gl_prog, "u_color");
  ctx->u_alpha = XglGetUniformLocation(ctx->gl_prog, "u_alpha");

  /* setup attributes */
  ctx->a_pos = XglGetAttribLocation(ctx->gl_prog, "a_pos");
  glVertexAttribPointer(ctx->a_pos, 3, GL_FLOAT, GL_FALSE,
                        3 * sizeof (GLfloat), cube_vertices_g);
  XglEnableVertexAttribArray(ctx->a_pos);

  ctx->a_uv = XglGetAttribLocation(ctx->gl_prog, "a_uv");
  glVertexAttribPointer(ctx->a_uv, 2, GL_FLOAT, GL_FALSE,
                        2 * sizeof (GLfloat), cube_tex_coords_g);
  glEnableVertexAttribArray(ctx->a_uv);

  esMatrixLoadIdentity(&pers);
  esPerspective(&pers,
                80.0f,  /* fovy */
                (float)ctx->width / ctx->height, /* aspect */
                0.5f,   /*nearZ*/
                10.0f); /*farZ*/

  esMatrixLoadIdentity(&view);
  esTranslate(&view, 0.0, 0.0, -2.0);
  esMatrixMultiply(&ctx->view, &view, &pers);

  setup_texture(ctx);

  XglEnable(GL_CULL_FACE);
  XglClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  XglViewport(0, 0, ctx->width, ctx->height);

  XglClear(GL_COLOR_BUFFER_BIT);

  egl_swap_buffers(ctx->egl);
}

static void
draw_frame(context_t *ctx)
{
  ESMatrix model, m;
  float angle;

  XglClear(GL_COLOR_BUFFER_BIT);

  // Compute matrix.
  esMatrixLoadIdentity(&model);
  angle = 360.0f * (ctx->frame % 1000) / 1000;
  esRotate(&model, 3. * angle, 1.0f, 0.0f, 0.0f);
  esRotate(&model, 2. * angle, 0.0f, 1.0f, 0.0f);
  esRotate(&model, angle, 0.0f, 0.0f, 1.0f);

  esMatrixMultiply(&m, &model, &ctx->view);

  XglUniformMatrix4fv(ctx->u_mat, 1, GL_FALSE, (GLfloat *)&m.m[0][0]);
  XglUniform4f(ctx->u_color, 1.0f, 0.1f, 0.1f, 1.0f);
  XglUniform1f(ctx->u_alpha, 0.5f);

  glDrawElements(GL_TRIANGLES,
                 cube_num_indices_g, GL_UNSIGNED_INT, cube_indices_g);
}

static void
player_usage(void)
{
  fprintf(stderr, "\nUsage: gles2 [options]\n");
  fprintf(stderr, "  -h: show this help\n");
  fprintf(stderr, "  -f <n>: run n frames of shader(s)\n");
  fprintf(stderr, "  -W <n>: set window width to n\n");
  fprintf(stderr, "  -H <n>: set window height to n\n");
  fprintf(stderr, "\n");
}

static void
parse_cmdline(context_t *ctx, int argc, char *argv[])
{
  int i;
  int opt;

  while ((opt = getopt(argc, argv, "f:hH:W:")) != -1) {

    switch (opt) {

    case 'f':
      i = atoi(optarg);
      if (i <= 0) {
        fprintf(stderr,
                "ERROR: -f option takes a positive integer argument "
                "(got %i)\n", i);
        exit(EXIT_FAILURE);
      }
      ctx->frames = (unsigned int)i;
      break ;

    case 'h':
      player_usage();
      exit(EXIT_SUCCESS);
      break ;

    case 'H':
      ctx->height = atoi(optarg);
      break ;

    case 'W':
      ctx->width = atoi(optarg);
      break ;

    default:
      player_usage();
      exit(EXIT_FAILURE);
    }
  }
}

static void
render_loop(context_t *ctx)
{
  for (;;) {

    draw_frame(ctx);
    egl_swap_buffers(ctx->egl);

    ctx->frame++;

    if ((ctx->frames > 0) && (ctx->frame > ctx->frames))
      break ;
  }
}

static context_t ctx_g;

int
main(int argc, char *argv[])
{
  context_t *ctx;

  ctx = &ctx_g;
  parse_cmdline(ctx, argc, argv);

  fprintf(stderr, "\ngles2cube: starting\n\n");

  ctx->egl = egl_init(ctx->width, ctx->height, ctx->winxpos, ctx->winypos);
  ctx->width = ctx->egl->width;
  ctx->height = ctx->egl->height;

  setup(ctx);

  render_loop(ctx);

  cleanup(ctx);

  return (0);
}

/*
* Copyright (c) 2015-2019, Julien Olivain <juju@cotds.org>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* End-of-File */
