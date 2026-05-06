
#define N 1024 
#define SPEC_W 1024

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <complex.h>
#include <stdio.h>
#include "raylib.h"
#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define FFT_IMPLEMENTATION
#include "fft.h"

#define SIGNAL_IMPLEMENTATION 
#include "signal_utils.h"

#if defined(PLATFORM_WEB)
    #define GLSL_VERSION 100
#else
    #define GLSL_VERSION 330
#endif

const size_t sw = 1920;
const size_t sh = 1024;

typedef struct {
    bool img_loaded;
    Image img;
    float* data;
    Texture2D img_tex;
    Texture2D freq_tex;
} AppState;
static AppState app = {0};

void fftshift(float *in, float *out, int w, int h)
{
    int hw = w / 2;
    int hh = h / 2;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int sx = (x + hw) % w;
            int sy = (y + hh) % h;

            out[y*w + x] = in[sy*w + sx];
        }
    }
}

Image make_image_from_float(float *data, int width, int height) {
    Color *pixels = malloc(sizeof *pixels * width * height);
    assert(pixels != NULL);

    float max_v = data[0];
    for (int i = 0; i < width * height; i++) {
        if(max_v < data[i])
            max_v = data[i];
    }

    for (int i = 0; i < width * height; i++) {
        float v = data[i];
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;

        unsigned char c = (unsigned char)(v * 255.0f);

        pixels[i] = (Color){ c, c, c, 255 };
    }

    Image img = {
        .data = pixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return img;
}
void app_init(){
    InitWindow(sw, sh, "Spectrum Play");
}

void app_deinit(){
    CloseWindow();
}

void update_frame() {
    BeginDrawing();
    ClearBackground(BLACK);

    if (app.img_loaded) {
        DrawTexture(app.img_tex, 0, 0, WHITE);
        DrawTexture(app.freq_tex, 512, 0, WHITE);
    }

    DrawFPS(50, 50);
    EndDrawing();
}
void abs_spec_img(const float complex spec[], float out[], size_t n)
{
    float maxv = -1e9f;
    for (size_t i = 0; i < n; i++) {
        float v = 20.0f * log10f(cabsf(spec[i]) + 1e-6f);
        out[i] = v;
        if (v > maxv) maxv = v;
    }

    float minv = maxv - 80.0f; // 80 dB zakres

    for (size_t i = 0; i < n; i++) {
        float v = (out[i] - minv) / (maxv - minv);

        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;

        // opcjonalnie kontrast

        out[i] = v;
    }
}

void draw_scene(){
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_frame, 0, 1);
#else
    while (!WindowShouldClose()) update_frame();
#endif
}

void load_img(const char* img_path){
    app.img = LoadImage(img_path);
    Color *pixels = LoadImageColors(app.img);
    app.img_loaded = true;
    int width = app.img.width;
    int height = app.img.height;

    float *data = malloc(sizeof(float) * width*height);
    float complex *freq = malloc(sizeof(float complex) * width*height);
    float  *freq_view = malloc(sizeof(float) * width*height);
    for(size_t dy = 0; dy < app.img.height; dy++){
        for(size_t dx = 0; dx < app.img.width; dx++){
            int i = (dy * width) + dx;
            data[i] = (0.299f * pixels[i].r +
                0.587f * pixels[i].g +
                0.114f * pixels[i].b) / 255.0f;
        }
    }

    app.data = data;
    app.img_tex = LoadTextureFromImage(app.img);
    printf("[APP] width %d, height %d \n", width, height);
    fft_img(data, freq, width, height);
    float *freq_abs = malloc(sizeof(float) * width*height);
    abs_spec_img(freq, freq_abs, width * height);
    fftshift(freq_abs, freq_view, width, height);

    Image freq_img = make_image_from_float(freq_view, width, height);
    app.freq_tex = LoadTextureFromImage(freq_img);
}


int main(int argc, char **argv){
    const char* img_path;
    for (size_t i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    img_path = "./image/lena.png";

    if(argc > 1){
        img_path = argv[1] ;
    }
    
    printf("img PATH %s\n\n", img_path);

    app_init();
    load_img(img_path);
    draw_scene();
    app_deinit();
    return 0;
}

