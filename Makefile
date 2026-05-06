# -------- COMPILER --------
CC := cc

# -------- COMMON --------
RAYLIB_INC := third_party/raylib/include
RAYLIB_LIB_DESKTOP := third_party/raylib/lib/libraylib.a
RAYLIB_LIB_WEB := third_party/raylib/lib/libraylib.web.a

DEPFLAGS := -MMD -MP

# -------- DESKTOP --------
CFLAGS := -O2 -Wall -Wextra -I$(RAYLIB_INC) -DPLATFORM_DESKTOP
CFLAGS_DEBUG := -g -O0 -Wall -Wextra -I$(RAYLIB_INC) -DPLATFORM_DESKTOP

LDFLAGS := $(RAYLIB_LIB_DESKTOP) \
	-lm -ldl -lpthread \
	-lGL -lX11 -lXrandr -lXi -lXcursor -lXinerama

# -------- SOURCES --------
SRC := main.c
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)

SRC_IMG := fft_img.c
OBJ_IMG := $(SRC_IMG:.c=.o)
DEP_IMG := $(OBJ_IMG:.o=.d)

# -------- TARGETS --------
.PHONY: all clean debug web serve

all: game img

game: $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

img: $(OBJ_IMG)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# -------- DEBUG --------
debug: game_debug

main_debug.o: main.c
	$(CC) $(CFLAGS_DEBUG) $(DEPFLAGS) -MF main_debug.d -c $< -o $@

game_debug: main_debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

# -------- WEB --------
EMSDK_FLAGS := -Os \
	-s USE_GLFW=3 \
	-s ASYNCIFY \
	-s TOTAL_MEMORY=67108864 \
	-s EXPORTED_FUNCTIONS='["_main","_load_music_web","_set_bars_web","_set_color_schema_web","_set_gain_web","_malloc","_push_data"]' \
	-s EXPORTED_RUNTIME_METHODS='["ccall","FS"]' \
	-DPLATFORM_WEB \
	--shell-file $(CURDIR)/shell.html \
	--preload-file resources

WEB_TARGET := index.html

web: $(WEB_TARGET)

$(WEB_TARGET): main.c $(wildcard *.h) shell.html
	@mkdir -p docs
	emcc $< \
		-o docs/$@ \
		-I$(RAYLIB_INC) \
		$(RAYLIB_LIB_WEB) \
		$(EMSDK_FLAGS)

serve: web
	cd docs && python3 -m http.server 8080

# -------- CLEAN --------
clean:
	rm -f game game_debug img *.o *.d
	rm -rf docs

-include $(DEP) $(DEP_IMG) main_debug.d
