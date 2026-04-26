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

typedef struct {
    RingBuffer samples;
    float complex *freq_complex;
    float spectrogram[SPEC_W][N/2];
    int spec_x;

    bool isMusicPlaying;
    bool musicLoaded;

    float window[N];
    float freq[N];
    int n_bars;
    int color_schema;

    int window_loc;
    int freq_loc;
    int n_bars_loc;
    int color_schema_loc1;
    int color_schema_loc2;
    Shader spectogram_shader;
    Shader audio_singal_shader;
    Shader spectrum_shader;

    RenderTexture2D  spectogram_texture; 
    RenderTexture2D freq_texture;
    RenderTexture2D audio_texture;
    Music music;
} AppState;
static AppState app = {0};

void audio_callback(void *bufferData, unsigned int frames){
    float *buffer = (float *)bufferData;

    for(int frame = 0; frame < frames; frame++)
        rb_write(&app.samples,  buffer[frame * 2 + 0]); //take only left channel
}

void test_impulse(){
    rb_write(&app.samples, 1);
    for(size_t i = 1; i < N; i++) rb_write(&app.samples, 0); 
}

void test_dc(){
    for(size_t i = 0; i < N; i++) rb_write(&app.samples, 1);
}

void test_sine_bin(){
    int k = 32;

    for(size_t i = 0; i < N; i++){
        rb_write(&app.samples, sinf(2.0f * M_PI * k * i / N));
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

        rb_write(&app.samples, sinf(phase));
    }
}
void app_init(){
    InitWindow(sw, sh, "Fourier Transform");
    InitAudioDevice();

    app.spectogram_texture = LoadRenderTexture(SPEC_W, N/2);
    app.freq_texture = LoadRenderTexture(N/2, 1);
    app.audio_texture = LoadRenderTexture(N, 1);


    app.spectogram_shader = LoadShader(0, TextFormat("resources/glsl%i/spectogram.fs", GLSL_VERSION));
    app.audio_singal_shader = LoadShader(0, TextFormat("resources/glsl%i/audio_signal.fs", GLSL_VERSION));
    app.spectrum_shader = LoadShader(0, TextFormat("resources/glsl%i/spectrum_shader.fs", GLSL_VERSION));

    app.window_loc = GetShaderLocation(app.audio_singal_shader, "window");
    app.freq_loc = GetShaderLocation(app.spectrum_shader, "freq");
    app.n_bars_loc = GetShaderLocation(app.spectrum_shader, "n_bars");

    app.color_schema_loc1 = GetShaderLocation(app.spectrum_shader, "color_schema");
    app.color_schema_loc2 = GetShaderLocation(app.spectogram_shader, "color_schema");
    app.n_bars = 32;
    app.color_schema = 0;
    app.spec_x = 0;

    app.music.looping = false; 
    app.samples = rb_init(N);
    app.freq_complex = malloc(sizeof(float complex) * N);

    Shader shaders[] = { app.spectogram_shader, app.spectrum_shader, app.audio_singal_shader};
    size_t n_shaders = sizeof(shaders) / sizeof(shaders[0]);
    for(int i = 0; i < n_shaders; i++){
        Shader test = shaders[i];
        if(!IsShaderValid(test)){
            fprintf(stderr, "Invalid shader syntax! %d", i);
            return;
        }
    } 
}

void load_music(const char *audio_path){
    FileMusic file = open_file_music(audio_path);    
    app.music = LoadMusicStreamFromMemory(file.format, file.data, file.byte_size); 
    app.musicLoaded = true;


}
void app_deinit(){
    DetachAudioStreamProcessor(app.music.stream, audio_callback);
    CloseAudioDevice();
    CloseWindow();
}

void update_frame(){
    UpdateMusicStream(app.music);
    BeginDrawing();
    ClearBackground(BLACK);
    if (IsKeyPressed(KEY_ONE))   app.color_schema = 0;
    if (IsKeyPressed(KEY_TWO))   app.color_schema = 1;
    if (IsKeyPressed(KEY_THREE)) app.color_schema = 2;
    if (IsKeyPressed(KEY_FOUR))  app.color_schema = 3;
    if(IsKeyPressed(KEY_SPACE)){
        app.isMusicPlaying = !app.isMusicPlaying;
        if(app.isMusicPlaying)
            PauseMusicStream(app.music);
        else 
            PlayMusicStream(app.music);
    }

    if(IsKeyPressed(KEY_A)){
        app.n_bars*=2;
        if(app.n_bars > N/2) 
            app.n_bars = N/2;
    }
    if(IsKeyPressed(KEY_D)){
        app.n_bars/=2;
        if(app.n_bars <= 2) app.n_bars = 2;
    }
    for(int i = 0; i < N; i++) {
        float data = rb_read(&app.samples, i);
        float w = 1;

        float x=0;
        //hann window apply
        w = 0.5f * (1 - cosf((2 * PI * i) / (N - 1)));

        app.window[i] = data * w;
    }

    fft(app.window, app.freq_complex, 1, N);
    abs_spectrum(app.freq_complex, app.freq, N/2);
    for(int i = 0; i < N/2; i++){
        float v = app.freq[i];
        float freqNorm = (float)i / (N/2);
        v *= powf(freqNorm, 0.4f);
        app.freq[i] = v;
    }

    SetShaderValueV(app.audio_singal_shader, app.window_loc, app.window, SHADER_UNIFORM_FLOAT, N);
    SetShaderValueV(app.spectrum_shader, app.freq_loc, app.freq, SHADER_UNIFORM_FLOAT, N/2);
    SetShaderValue(app.spectrum_shader, app.n_bars_loc, &app.n_bars, SHADER_UNIFORM_INT);

    SetShaderValue(app.spectrum_shader, app.color_schema_loc1, &app.color_schema, SHADER_UNIFORM_INT);
    SetShaderValue(app.spectogram_shader, app.color_schema_loc2, &app.color_schema, SHADER_UNIFORM_INT);

    for(int i = 0; i < N/2; i++){
        app.spectrogram[app.spec_x][i] = app.freq[i];
    }

    if(app.samples.write_index > N){
        BeginTextureMode(app.spectogram_texture);
        for (int y = 0; y < N/2; y++) {
            float v = app.spectrogram[app.spec_x][y];
            //v = powf(v, 0.4f);
            v*=255;
            DrawPixel(app.spec_x, (N/2 - 1 - y), (Color){v, v, v, 255});
        }
        app.spec_x = (app.spec_x + 1 ) % SPEC_W;
        EndTextureMode();
    }

    BeginShaderMode(app.spectrum_shader);
    DrawTexturePro(
            app.freq_texture.texture,
            (Rectangle){0, 0, N/2, -1},
            (Rectangle){sw/2, 0, sw/2, sh/2},
            (Vector2){0,0},
            0,
            WHITE
            );
    EndShaderMode();


    BeginShaderMode(app.audio_singal_shader);
    DrawTexturePro(
            app.audio_texture.texture,
            (Rectangle){0, 0, N, 1},
            (Rectangle){0, 0, sw/2, sh/2},
            (Vector2){0,0},
            0,
            WHITE
            );
    EndShaderMode();


    BeginShaderMode(app.spectogram_shader);
    DrawTexturePro(
            app.spectogram_texture.texture,
            (Rectangle){0, 0, SPEC_W, -(N/2)},  
            (Rectangle){0, sh/2, sw, sh/2},
            (Vector2){0,0},
            0,
            WHITE
            );
    EndShaderMode();
    DrawFPS(50, 50);
    EndDrawing();

}
void draw_scene(){
    if(app.musicLoaded){
        PlayMusicStream(app.music);
        AttachAudioStreamProcessor(app.music.stream, audio_callback);
    }
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_frame, 0, 1);
#else
    while (!WindowShouldClose()) update_frame();
#endif
}


int main(int argc, char **argv){
    const char* audio_path;

    for (int i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    //audio_path = "./audio/arctic.mp3";
    audio_path = "./audio/bass.wav";

    if(argc > 1){
        audio_path = argv[1] ;
    }
    
    printf("AUDIO PATH %s\n\n", audio_path);

    app_init();

    load_music(audio_path);
    draw_scene();
    app_deinit();
    return 0;
}

