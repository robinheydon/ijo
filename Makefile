SRCS = $(wildcard src/*.c)
DEPS = $(wildcard src/*.h)
ZSRCS = $(wildcard src/*.zig)

DEBUG_OBJS = $(patsubst src/%.c, _build/debug/%.o, $(SRCS))
RELEASE_OBJS = $(patsubst src/%.c, _build/release/%.o, $(SRCS))

ZOBJS = $(patsubst src/%.zig, _build/%.o, $(ZSRCS))

DEBUG_EXE = city_debug
RELEASE_EXE = city

CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -pedantic
CFLAGS += -I src
CFLAGS += -I libs/flecs
CFLAGS += -I libs/raylib/src

DEBUG_CFLAGS += $(CFLAGS)
DEBUG_CFLAGS += -g

RELEASE_CFLAGS += $(CFLAGS)
RELEASE_CFLAGS += -O2

ZIGFLAGS +=

FLECS_OBJS += _build/flecs/flecs.o

FLECS_CFLAGS += -O2

RAYLIB_OBJS += _build/raylib/raudio.o
RAYLIB_OBJS += _build/raylib/rcore.o
RAYLIB_OBJS += _build/raylib/rglfw.o
RAYLIB_OBJS += _build/raylib/rmodels.o
RAYLIB_OBJS += _build/raylib/rshapes.o
RAYLIB_OBJS += _build/raylib/rtext.o
RAYLIB_OBJS += _build/raylib/rtextures.o
RAYLIB_OBJS += _build/raylib/utils.o

RAYLIB_CFLAGS += -DPLATFORM_DESKTOP
RAYLIB_CFLAGS += -O2

debug : $(DEBUG_EXE)
release : $(RELEASE_EXE)

run : $(DEBUG_EXE)
	./$(DEBUG_EXE)

run_release : $(RELEASE_EXE)
	./$(RELEASE_EXE)

$(DEBUG_EXE) : $(DEBUG_OBJS) $(ZOBJS) $(FLECS_OBJS) $(RAYLIB_OBJS)
	zig cc -o $@ $(DEBUG_OBJS) $(ZOBJS) $(FLECS_OBJS) $(RAYLIB_OBJS)

$(RELEASE_EXE) : $(RELEASE_OBJS) $(ZOBJS) $(FLECS_OBJS) $(RAYLIB_OBJS)
	zig cc -o $@ $(RELEASE_OBJS) $(ZOBJS) $(FLECS_OBJS) $(RAYLIB_OBJS)

_build/debug/%.o : src/%.c $(DEPS) Makefile | _build
	zig cc -c -o $@ $< $(DEBUG_CFLAGS)

_build/release/%.o : src/%.c $(DEPS) Makefile | _build
	zig cc -c -o $@ $< $(RELEASE_CFLAGS)

_build/%.o : src/%.zig Makefile | _build
	zig build-obj -femit-bin=$@ $< $(ZIGFLAGS)

_build/flecs/flecs.o : libs/flecs/flecs.c Makefile | _build
	zig cc -c -o $@ $< $(FLECS_CFLAGS)

_build/raylib/%.o : libs/raylib/src/%.c Makefile | _build
	zig cc -c -o $@ $< $(RAYLIB_CFLAGS)

_build/raylib/platforms/%.o : libs/raylib/src/external/glfw/src/platforms/%.c Makefile | _build
	zig cc -c -o $@ $< $(RAYLIB_CFLAGS)

_build/raylib/glfw/%.o : libs/raylib/src/external/glfw/src/%.c Makefile | _build
	zig cc -c -o $@ $< $(RAYLIB_CFLAGS)

_build :
	@-mkdir _build
	@-mkdir _build/release
	@-mkdir _build/debug
	@-mkdir _build/flecs
	@-mkdir _build/raylib
	@-mkdir _build/raylib/glfw
	@-mkdir _build/raylib/platforms

clean :
	rm -rf _build

.SUFFIXES :

MAKEFLAGS += --no-builtin-rules
