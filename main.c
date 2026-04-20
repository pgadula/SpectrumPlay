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

RingBuffer samples;
float complex *freq_complex;
float spectrogram[SPEC_W][N/2];
int spec_x = 0;

void audio_callback(void *bufferData, unsigned int frames){
    float *buffer = (float *)bufferData;

    for(int frame = 0; frame < frames; frame++)
        rb_write(&samples,  buffer[frame * 2 + 0]); //take only left channel
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

void test_impulse(){
    rb_write(&samples, 1);
    for(size_t i = 1; i < N; i++) rb_write(&samples, 0); 
}

void test_dc(){
    for(size_t i = 0; i < N; i++) rb_write(&samples, 1);
}

void test_sine_bin(){
    int k = 32;

    for(size_t i = 0; i < N; i++){
        rb_write(&samples, sinf(2.0f * M_PI * k * i / N));
    }
}

void sweapy(){
    float phase = 0.0f;
    float f0 = 2.0f;
    float f1 = 500.0f;

    for (int i = 0; i < N; i++) {
        float t = (float)i / N;
        float f = f0 + (f1 - f0) * t;

        phase += 2.0f * M_PI * f / N; // integracja częstotliwości

        rb_write(&samples, sinf(phase));
    }
}

void draw_scene(const FileMusic *file){
        float window[N];
        float freq[N];

        InitWindow(sw, sh, "Fourier Transform");
        InitAudioDevice();

        RenderTexture2D spectogram_texture = LoadRenderTexture(SPEC_W, N/2);
        RenderTexture2D freq_texture = LoadRenderTexture(N/2, 1);
        RenderTexture2D audio_texture = LoadRenderTexture(N, 1);

        Music m = LoadMusicStreamFromMemory(file->format, file->data, file->byte_size); // Load music stream from data

        Shader spectogram_shader = LoadShader(0, TextFormat("resources/glsl%i/spectogram.fs", GLSL_VERSION));
        Shader audio_singal_shader = LoadShader(0, TextFormat("resources/glsl%i/audio_signal.fs", GLSL_VERSION));
        Shader spectrum_shader = LoadShader(0, TextFormat("resources/glsl%i/spectrum_shader.fs", GLSL_VERSION));

        int window_loc = GetShaderLocation(audio_singal_shader, "window");
        int freq_loc = GetShaderLocation(spectrum_shader, "freq");


        Shader shaders[] = { spectogram_shader, spectrum_shader, audio_singal_shader};
        size_t n_shaders = sizeof(shaders) / sizeof(shaders[0]);
        for(int i = 0; i < n_shaders; i++){
            Shader test = shaders[i];
            if(!IsShaderValid(test)){
                fprintf(stderr, "Invalid shader syntax! %d", i);
                return;
            }
        } 
         
        m.looping = false; 

        samples = rb_init(N);

        freq_complex = malloc(sizeof(float complex) * N);

        PlayMusicStream(m);

        //sweapy();
        //test_impulse();
        //test_dc();
        //test_sine_bin();


        AttachAudioStreamProcessor(m.stream, audio_callback);
        while (!WindowShouldClose())
        {
            UpdateMusicStream(m);
            BeginDrawing();
            ClearBackground(BLACK);

            for(int i = 0; i < N; i++) {
                float data = rb_read(&samples, i);

                //hann window apply
                float w = 0.5f * (1 - cosf((2 * PI * i) / (N - 1)));
                window[i] = data * w;
            }

            fft(window, freq_complex, 1, N);
            abs_spectrum(freq_complex, freq, N);

            SetShaderValueV(audio_singal_shader, window_loc, window, SHADER_UNIFORM_FLOAT, N);
            SetShaderValueV(spectrum_shader, freq_loc, freq, SHADER_UNIFORM_FLOAT, N/2);

            for(int i = 0; i < N/2; i++){
                spectrogram[spec_x][i] = freq[i];
            }



            if(samples.write_index > N){
                BeginTextureMode(spectogram_texture);
                for (int y = 0; y < N/2; y++) {
                    float v = spectrogram[spec_x][y];
                    v = powf(v, 0.4f);
                    Color c = inferno(v);
                    DrawPixel(spec_x, (N/2 - 1 - y), c);
                }
                spec_x = (spec_x + 1 ) % SPEC_W;
                EndTextureMode();
            }

            BeginShaderMode(spectrum_shader);
                DrawTexturePro(
                        freq_texture.texture,
                        (Rectangle){0, 0, N/2, 1},
                        (Rectangle){sw/2, 0, sw/2, sh/2},
                        (Vector2){0,0},
                        0,
                        WHITE
                        );
            EndShaderMode();


            BeginShaderMode(audio_singal_shader);
            DrawTexturePro(
                    audio_texture.texture,
                    (Rectangle){0, 0, N, 1},
                    (Rectangle){0, 0, sw/2, sh/2},
                    (Vector2){0,0},
                        0,
                        WHITE
                    );
            EndShaderMode();


            BeginShaderMode(spectogram_shader);
                DrawTexturePro(
                        spectogram_texture.texture,
                        (Rectangle){0, 0, SPEC_W, -(N/2)},  
                        (Rectangle){0, sh/2, sw, sh/2},
                        (Vector2){0,0},
                        0,
                        WHITE
                        );
            EndShaderMode();
            EndDrawing();
        }


        DetachAudioStreamProcessor(m.stream, audio_callback);
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

