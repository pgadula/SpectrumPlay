#define N 1024 
#define LOG_N 32 
#define M_2PI  M_PI*2
#define SPEC_W 800

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <complex.h>
#include <stdio.h>
#include "raylib.h"
#include <math.h>

#define RB_IMPLEMENTATION
#include "ring_buffer.h"

#define FILE_MUSIC_UTILS_IMPLEMENTATION
#include "music_file_utils.h"

#define SIGNAL_IMPLEMENTATION 
#include "signal_utils.h"

#define FFT_IMPLEMENTATION
#include "fft.h"

#define GLSL_VERSION            330

const size_t sw = 1680;
const size_t sh = 1024;
const size_t half_sh = sh/2;
const size_t r = 5;

void example(float in[], size_t n){
    int F = 1;
    for(size_t i = 0; i < n; i++){
        float v1, v2;
        float t = (float)i/(float)N;
        v1 = sin(t*M_2PI*F);
        v2 = sin(t*M_2PI*F);
        float v = v1 + v2;
        in[i] = v;
    }
}


void gen_points(float in[], Vector2 points[], size_t n, float max, float min){
    float max_abs = fmax(fabs(max), fabs(min));
    for(size_t i = 0; i < n; i++){
        float p = in[i] / max_abs;
        points[i] = (Vector2){ .x = sw*i+r, .y = half_sh + (p*(half_sh-r))};
    }
}

void draw_signal_norm_area(const float in[], size_t n,
        float x0, float y0,
        float w, float h,
        Color col)
{
    if (n < 2) return;

    float dx = w / (float)(n - 1);
    float cy = y0 + h * 0.5f;
    float ay = h * 0.5f;

    for (size_t i = 0; i < n - 1; i++) {
        float p1 = in[i];
        float p2 = in[i + 1];

        float x1 = x0 + dx * (float)i;
        float x2 = x0 + dx * (float)(i + 1);

        float y1 = cy - p1 * ay;
        float y2 = cy - p2 * ay;

        DrawLineEx((Vector2){(int)x1, (int)y1}, (Vector2){(int)x2, (int)y2}, 3, col);
    }
}


void draw_spectrum_rects(const float values[], size_t n,
        float x0, float y0, float w, float h,
        Color fill, Color outline)
{
    if (n == 0) return;

    float barWf = w / (float)n;
    if (barWf < 1.0f) barWf = 1.0f;

    for (size_t i = 0; i < n; i++) {
        float a = values[i];
        if (a < 0.0f) a = 0.0f;
        if (a > 1.0f) a = 1.0f;

        float barH = a * h;

        float x = x0 + (float)i * barWf;
        float y = (y0 + h) - barH;

        int xi = (int)x;
        int yi = (int)y;
        int wi = (int)(barWf - 1.0f);
        if (wi < 1) wi = 1;
        int hi = (int)barH;
        DrawRectangle(xi, yi, wi, hi, fill);
        DrawRectangleLines(xi, yi, wi, hi, outline);
    }
}


void draw_singal(float signal[], int n){
    float max, min;
    Vector2 points[n];
    get_min_max(signal, n-1, &min, &max);
    gen_points(signal, points, n, max, min);

    for(size_t i = 0; i < n-1; i++){
        Vector2 p1 = points[i];
        Vector2 p2 = points[i+1];
        DrawLineV(p1, p2, RED);
    }

    for(size_t i = 0; i < n; i++){
        Vector2 p = points[i];
        DrawCircleV(p, r, GREEN);
    }
}


RingBuffer samples;
float complex *freq_complex;
int current_size;
int total_samples;
int current_index = 0;
float spectrogram[SPEC_W][N/2];
int spec_x = 0;

void audio_callback(void *bufferData, unsigned int frames){
    current_size = frames;
    float *buffer = (float *)bufferData;

    for(int frame = 0; frame < frames; frame++)
        rb_write(&samples,  buffer[frame * 2 + 0]); //take only left channel

    current_index += frames;
}

typedef struct {
    float t;
    float r, g, b;
} Stop;

Color inferno(float t){
    // clamp
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    static const Stop stops[] = {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {0.2f, 0.1f, 0.0f, 0.3f}, // purple
        {0.4f, 0.5f, 0.0f, 0.2f}, // red
        {0.6f, 0.9f, 0.3f, 0.0f}, // orange
        {0.8f, 1.0f, 0.7f, 0.0f}, // yellow-orange
        {1.0f, 1.0f, 1.0f, 0.8f}  // bright
    };

    const int count = sizeof(stops) / sizeof(stops[0]);

    for (int i = 0; i < count - 1; i++) {
        if (t >= stops[i].t && t <= stops[i + 1].t) {

            float range = stops[i + 1].t - stops[i].t;
            float local = (t - stops[i].t) / range;

            float r = stops[i].r + local * (stops[i + 1].r - stops[i].r);
            float g = stops[i].g + local * (stops[i + 1].g - stops[i].g);
            float b = stops[i].b + local * (stops[i + 1].b - stops[i].b);

            return (Color){
                (unsigned char)(r * 255.0f),
                (unsigned char)(g * 255.0f),
                (unsigned char)(b * 255.0f),
                255
            };
        }
    }

    return (Color){0, 0, 0, 255};
}
void draw_spectogram(int start_x, int start_y){
    for (int x = 0; x < SPEC_W; x++) {
        int idx = (spec_x + x) % SPEC_W;

        for (int y = 0; y < N/2; y++) {
            float v = spectrogram[idx][y];
            v = powf(v, 0.4f);
            DrawPixel(x+start_x, start_y + (N/2 - y), inferno(v));
        }
    }
}


void draw_scene(const FileMusic *file){
        float windows[N];
        float freq[N];

        InitWindow(sw, sh, "Fourier Transform");
        InitAudioDevice();

        RenderTexture2D spectogram_texture = LoadRenderTexture(SPEC_W, N/2);
        Music m = LoadMusicStreamFromMemory(file->format, file->data, file->byte_size); // Load music stream from data

        Shader spectogram_shader = LoadShader(0, TextFormat("resources/glsl%i/spectogram.fs", GLSL_VERSION));

        if(!IsShaderValid(spectogram_shader)){
            fprintf(stderr, "Invalid shader syntax!");
            return;
        }
         
        m.looping = false; 

        total_samples = m.frameCount;
        samples = rb_init(N);
        freq_complex = malloc(sizeof(float complex) * N);

        PlayMusicStream(m);
        AttachAudioStreamProcessor(m.stream, audio_callback);

        while (!WindowShouldClose())
        {

            UpdateMusicStream(m);
            BeginDrawing();
            ClearBackground(BLACK);

            for (int i = 0; i < N; i++) {
                float data = rb_read(&samples, i);

                //hann window apply
                float w = 0.5f * (1 - cosf((2 * PI * i) / (N - 1)));
                windows[i] = data * w;
            }

            fft(windows, freq_complex, 1, N);
            abs_spectrum(freq_complex, freq, N);

            draw_signal_norm_area(windows, N, 0, sh/2, sw/2, sh/2, WHITE);
            draw_spectrum_rects(freq, N/2, 5, 160, sw, sh/3, BLUE, BLUE);

            for(int i = 0; i < N/2; i++){
                spectrogram[spec_x][i] = 0.8f * spectrogram[spec_x][i] + 0.2f * freq[i];
            }

            BeginTextureMode(spectogram_texture);
                for (int y = 0; y < N/2; y++) {
                    float v = spectrogram[spec_x][y];
                    v = powf(v, 0.4f);
                    Color c = inferno(v);
                    DrawPixel(SPEC_W - spec_x, (N/2 - y), c);
                }
                spec_x = (spec_x + 1 ) % SPEC_W;
            EndTextureMode();

            BeginShaderMode(spectogram_shader);
                DrawTexturePro(
                        spectogram_texture.texture,
                        (Rectangle){0, 0, SPEC_W, -(N/2)},  
                        (Rectangle){sw/2, sh/2, sw/2, sh/2},
                        (Vector2){0,0},
                        0,
                        WHITE
                        );
            EndShaderMode();
            EndDrawing();
        }

        CloseAudioDevice();
        CloseWindow();
    }


int main(int argc, char **argv){
    const char* audio_path;

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    audio_path = "./audio/arctic.mp3";
    //audio_path = "./audio/bass.wav";

    if(argc > 1){
        audio_path = argv[1] ;
    }
    
    printf("AUDIO PATH %s\n\n", audio_path);

    FileMusic fm = open_file_music(audio_path);    
    draw_scene(&fm);
    return 0;
}

