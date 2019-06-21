AM_CFLAGS = -W -Wall -Wextra -Wshadow -Wpointer-arith -Wmissing-prototypes -Wcast-align \
		${egl_CFLAGS} ${glesv2_CFLAGS} \
		${x11_CFLAGS}

LDADD = \
	${egl_LIBS} ${glesv2_LIBS} \
	${x11_LIBS} -lm

if ENABLE_WL
AM_CFLAGS += ${wayland_client_CFLAGS} ${wayland_egl_CFLAGS}
LDADD += ${wayland_client_LIBS} ${wayland_egl_LIBS}
endif

if ENABLE_KMS
AM_CFLAGS += ${drm_CFLAGS} ${gbm_CFLAGS}
LDADD += ${drm_LIBS} ${gbm_LIBS}
endif

if ENABLE_SDL2
AM_CFLAGS += ${sdl2_CFLAGS}
LDADD += ${sdl2_LIBS}
endif

bin_PROGRAMS = gles2cube

gles2cube_SOURCES = \
  gles2cube.c gles2cube.h \
  egl_helper.c egl_helper.h \
  gles_helper.c gles_helper.h \
  esTransform.c esUtil.h \
  native_gfx.h

if ENABLE_X11
gles2cube_SOURCES += native_gfx_x11.c
endif

if ENABLE_VIVFB
gles2cube_SOURCES += native_gfx_vivfb.c
endif

if ENABLE_RPI
gles2cube_SOURCES += native_gfx_rpi.c
endif

if ENABLE_WL
gles2cube_SOURCES += native_gfx_wl.c
if ENABLE_IVI
gles2cube_SOURCES += ivi-application-protocol.c
endif
endif

if ENABLE_KMS
gles2cube_SOURCES += native_gfx_kms.c
endif

if ENABLE_WGL
gles2cube_SOURCES += native_gfx_wgl.c
endif

if ENABLE_TISGX
gles2cube_SOURCES += native_gfx_tisgx.c
endif

if ENABLE_MALI
gles2cube_SOURCES += native_gfx_mali.c
endif
