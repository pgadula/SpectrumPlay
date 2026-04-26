# -------- COMPILER --------
CC := cc

# -------- COMMON --------
RAYLIB_INC := third_party/raylib/include
RAYLIB_LIB_DESKTOP := third_party/raylib/lib/libraylib.a
RAYLIB_LIB_WEB := third_party/raylib/lib/libraylib.web.a

# -------- DESKTOP --------
CFLAGS := -O2 -Wall -Wextra -I$(RAYLIB_INC) -DPLATFORM_DESKTOP
CFLAGS_DEBUG := -g -O0 -Wall -Wextra -I$(RAYLIB_INC) -DPLATFORM_DESKTOP

LDFLAGS := $(RAYLIB_LIB_DESKTOP) \
	-lm -ldl -lpthread \
	-lGL -lX11 -lXrandr -lXi -lXcursor -lXinerama

# -------- TARGETS --------
all: game

game: main.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

# -------- DEBUG --------
debug: game_debug

game_debug: main_debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

main_debug.o: main.c
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

# -------- WEB --------
EMSDK_FLAGS := -Os \
	-s USE_GLFW=3 \
	-s ASYNCIFY \
	-s TOTAL_MEMORY=67108864 \
	-s EXPORTED_FUNCTIONS='["_main","_load_music_web", "_malloc"]' \
	-s EXPORTED_RUNTIME_METHODS='["ccall","FS"]' \
	-DPLATFORM_WEB \
	--shell-file $(CURDIR)/shell.html \
	--preload-file resources
WEB_TARGET := index.html

.PHONY: web serve

web: $(WEB_TARGET)

$(WEB_TARGET): main.c
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
	rm -f game game_debug *.o
	rm -rf docs
