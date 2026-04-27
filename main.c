#define N 1024 
#define SPEC_W 1024

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <complex.h>
#include <stdio.h>
#include "raylib.h"
#include "test_signal.h"
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


#if defined(PLATFORM_WEB)
    #define GLSL_VERSION 100
#else
    #define GLSL_VERSION 330
#endif

const size_t sw = 1920;
const size_t sh = 1024;

typedef struct {
    RingBuffer samples;
    float complex *freq_complex;
    float spectrogram[SPEC_W][N/2];
    int spec_x;

    bool isMusicPlaying;
    bool musicLoaded;

    float gain;
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

    Texture2D spectogram_texture;
    Color spec_column[N/2];
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

void app_init(){
    InitWindow(sw, sh, "Fourier Transform");
    InitAudioDevice();

    app.gain = 1.0;
    Image spec_img = GenImageColor(SPEC_W, N/2, BLACK);
    app.spectogram_texture = LoadTextureFromImage(spec_img);
    UnloadImage(spec_img);
    SetTextureFilter(app.spectogram_texture, TEXTURE_FILTER_POINT);

    app.freq_texture = LoadRenderTexture(N/2, 1);
    app.audio_texture = LoadRenderTexture(N, 1);

    BeginTextureMode(app.freq_texture);
    ClearBackground(BLACK);
    EndTextureMode();

    BeginTextureMode(app.audio_texture);
    ClearBackground(BLACK);
    EndTextureMode();


    app.spectogram_shader = LoadShader(0, TextFormat("resources/glsl%i/spectogram.fs", GLSL_VERSION));
    app.audio_singal_shader = LoadShader(0, TextFormat("resources/glsl%i/audio_signal.fs", GLSL_VERSION));
    app.spectrum_shader = LoadShader(0, TextFormat("resources/glsl%i/spectrum_shader.fs", GLSL_VERSION));

    app.window_loc = GetShaderLocation(app.audio_singal_shader, "window");
    app.freq_loc = GetShaderLocation(app.spectrum_shader, "freq");
    app.n_bars_loc = GetShaderLocation(app.spectrum_shader, "n_bars");

    app.color_schema_loc1 = GetShaderLocation(app.spectrum_shader, "color_schema");
    app.color_schema_loc2 = GetShaderLocation(app.spectogram_shader, "color_schema");
    app.n_bars = 32;
    app.color_schema = 4;
    app.spec_x = 0;

    app.music.looping = false; 
    app.samples = rb_init(N*3);
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


// ###
//   ###
//     ###
//       ###
//
// 
// w = 3  
// idx
//         0-3
//         2-5
//         4-7

void load_music(const char *audio_path){
    if (app.musicLoaded) {
        StopMusicStream(app.music);
        DetachAudioStreamProcessor(app.music.stream, audio_callback);
        UnloadMusicStream(app.music);
        app.musicLoaded = false;
    }

    FileMusic file = open_file_music(audio_path);

    app.music = LoadMusicStreamFromMemory(
        file.format,
        file.data,
        file.byte_size
    );

    if (!IsMusicValid(app.music)) {
        fprintf(stderr, "Failed to load music: %s\n", audio_path);
        return;
    }

    app.music.looping = false;
    app.musicLoaded = true;
    app.isMusicPlaying = true;

    PlayMusicStream(app.music);
    AttachAudioStreamProcessor(app.music.stream, audio_callback);
}

#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE
#endif
void push_data(float *data, int count) {
    for (int i = 0; i < count; i++) {
        rb_write(&app.samples, data[i]);
    }
}

#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE
#endif
void set_gain_web(float value) {
    if (value < 0.1f) value = 0.1f;
    if (value > 10.0f) value = 10.0f;

    app.gain = value;
}

#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE
#endif
void set_bars_web(int value) {
    if (value < 2) value = 2;
    if (value > N / 2) value = N / 2;

    app.n_bars = value;
}

#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE
#endif
void set_color_schema_web(int value) {
    if (value < 0) value = 0;
    if (value > 3) value = 3;

    app.color_schema = value;
}
#if defined(PLATFORM_WEB)
EMSCRIPTEN_KEEPALIVE
#endif
void load_music_web(unsigned char *data, int size, const char *format) {

    if (app.musicLoaded) {
        StopMusicStream(app.music);
        DetachAudioStreamProcessor(app.music.stream, audio_callback);
        UnloadMusicStream(app.music);
        app.musicLoaded = false;
    }

    app.music = LoadMusicStreamFromMemory(format, data, size);

    if (!IsMusicValid(app.music)) {
        printf("Failed to load from bytes\n");
        return;
    }

    app.music.looping = false;
    app.musicLoaded = true;
    app.isMusicPlaying = true;

    PlayMusicStream(app.music);
    AttachAudioStreamProcessor(app.music.stream, audio_callback);
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

   // for(int i = 0; i < N; i++) {
   //     float data = rb_read(&app.samples, i);
   //     float w = 1;

   //     //hann window apply
   //     w = 0.5f * (1 - cosf((2 * PI * i) / (N - 1)));

   //     app.window[i] = data * w;
   // }
    rb_read_window(&app.samples, N, app.window);
    
    for(int i = 0; i < N; i++) {
        float w = 0.5f * (1 - cosf((2 * PI * i) / (N - 1)));
        app.window[i] *= w;

    }

//    dft(app.window, app.freq_complex, N);
    fft(app.window, app.freq_complex, 1, N);
    abs_spectrum(app.freq_complex, app.freq, N/2);
    for(int i = 0; i < N/2; i++){
        float v = app.freq[i];
        float freqNorm = (float)i / (N/2);
        v *= powf(freqNorm, 0.4f);
        v *= app.gain;
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
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

    if (app.samples.write_index > N) {
        int h = N / 2;

        for (int y = 0; y < h; y++) {
            float v = app.freq[y];

            if (v < 0.0f) v = 0.0f;
            if (v > 1.0f) v = 1.0f;

            unsigned char c = (unsigned char)(v * 255.0f);

            app.spec_column[h - 1 - y] = (Color){ c, c, c, 255 };
        }

    Rectangle col = {
        .x = app.spec_x,
        .y = 0,
        .width = 1,
        .height = h
    };

    UpdateTextureRec(app.spectogram_texture, col, app.spec_column);

    app.spec_x = (app.spec_x + 1) % SPEC_W;
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
            app.spectogram_texture,
            (Rectangle){0, 0, SPEC_W, -(N/2)},
            (Rectangle){0, sh/2, sw, sh/2},
            (Vector2){0,0},
            0,
            WHITE
            );
    EndShaderMode();
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
    audio_path = "./audio/sweep.wav";

    if(argc > 1){
        audio_path = argv[1] ;
    }
    
    printf("AUDIO PATH %s\n\n", audio_path);

    app_init();
#if !defined(PLATFORM_WEB)
    load_music(audio_path);
#endif

draw_scene();
    draw_scene();
    app_deinit();
    return 0;
}

