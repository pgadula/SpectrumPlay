CC      := cc
CFLAGS  := -O2 -Wall -Wextra -Ithird_party/raylib/include 
CFLAGS += -DPLATFORM_DESKTOP

CFLAGS_DEBUG := -g -O0 -Wall -Wextra -Ithird_party/raylib/include

LDFLAGS := third_party/raylib/lib/libraylib.a \
		   -lm -ldl -lpthread \
		   -lGL -lX11 -lXrandr -lXi -lXcursor -lXinerama -DPLATFORM_DESKTOP

all: game

game: main.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- DEBUG ---
debug: game_debug

game_debug: main_debug.o
	$(CC) -o $@ $^ $(LDFLAGS)

main_debug.o: main.c
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

clean:
	rm -f game game_debug *.o


RAYLIB_WEB_LIB := third_party/raylib/lib/libraylib.web.a
EMSDK_FLAGS := -Os -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -DPLATFORM_WEB --shell-file $(CURDIR)/shell.html
WEB_PROGS := fft.html

.PHONY: web serve

web: $(WEB_PROGS)

# -------- Web builds --------

fft.html: main.c
	@mkdir -p docs
	emcc -o docs/$@ $< -I$(RAYLIB_INC) $(RAYLIB_WEB_LIB) $(EMSDK_FLAGS)

serve: web
	cd docs && python3 -m http.server 8080

RAYLIB_WEB_LIB := third_party/raylib/lib/libraylib.web.a
EMSDK_FLAGS := -Os -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -DPLATFORM_WEB --shell-file $(CURDIR)/shell.html

WEB_PROGS := knn.html perceptron.html svm.html nonld.c

.PHONY: web serve

web: $(WEB_PROGS)
# -------- Web builds --------


fft.html: main.c
	@mkdir -p docs
	emcc -o docs/$@ $< -I$(RAYLIB_INC) $(RAYLIB_WEB_LIB) $(EMSDK_FLAGS)

