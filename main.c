#include <assert.h>
#include <stdlib.h>
#include <complex.h>
#include <stdio.h>
#include "raylib.h"
#include <math.h>
#define FFT_IMPLEMENTATION
#include "fft.h"
#define N 1024 
#define LOG_N 32 
#define M_2PI  M_PI*2
#define SPEC_W 800

const size_t sw = 1680;
const size_t sh = 1024;
const size_t half_sh = sh/2;
const size_t r = 5;
float zoom = 2;


void log_spectrum(const float in[], float out[], int n, int out_n)
{
    for (int i = 0; i < out_n; i++) out[i] = 0.0f;

    for (int i = 0; i < n; i++) {
        int bin = (int)(log2f(i + 1));
        if (bin >= out_n) bin = out_n - 1;

        if (in[i] > out[bin])
            out[bin] = in[i];
    }
}

void abs_spectrum(const float complex spec[],
                  float out[],
                  int n)
{
    for (int i = 0; i < n; i++) {
        float v = 20.0f * log10f(cabsf(spec[i]) + 1e-6f);

        v = (v + 80.0f) / 80.0f;

        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;

        out[i] = v;
    }
}

void print(float in[], size_t n){
    for(size_t i = 0; i < n; i++){
       printf("%f, ", in[i]);
    }
    printf("\n");
}

void print_freq(float complex freq[], size_t n){
    printf("\n");
    for(size_t f = 0; f < n; f++){
       printf("[%zu]\t r:%f i:%f\n", f, crealf(freq[f]), cimagf(freq[f]));
    }
}

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

void gen_signal(float out[], double (*func)(double t), int f, float p, size_t n){
    float ff = M_2PI * f;
    for(size_t i = 0; i < n; i++){
        float t = (float)i/(float)(n);
        float v = (*func)(t * ff + p);
        out[i] = v;
    }
}

void gen_signalI(float complex out[], float complex (*func)(float complex t), int f, float p, size_t n){
    float ff = M_2PI * f;
    for(size_t i = 0; i < n; i++){
        float t = (float)i/(float)(n-1);
        float v = (*func)(t * ff + p);
        out[i] = v;
    }
}

void get_min_max(float in[], size_t n, float *min, float *max){
    float c_min = in[0];
    float c_max = in[0];
    for(size_t i = 0; i < n; i++){
        float v = in[i];
        if(c_min > v){
            c_min = v;
        }
        if(c_max < v){
            c_max = v;
        }
    }
    (*min) = c_min;
    (*max) = c_max;
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


void spectrum_abs_normalize(const complex float in[], float out[], size_t n)
{
    float maxAbs = 0.0f;

    for (size_t i = 0; i < n; i++) {
        float a = fabsf(in[i]);
        if (a > maxAbs) maxAbs = a;
    }

    if (maxAbs <= 0.0f) {
        for (size_t i = 0; i < n; i++) out[i] = 0.0f;
        return;
    }

    for (size_t i = 0; i < n; i++) {
        out[i] = fabsf(in[i]) / maxAbs;
    }
}

void draw_spectrum_rects(const float values[], size_t n,
        float x0, float y0, float w, float h,
        Color fill, Color outline)
{
    if (n == 0) return;

    float barWf = w / (float)n;
    if (barWf < 1.0f) barWf = 1.0f;

   // DrawLine((int)x0, (int)(y0 + h), (int)(x0 + w), (int)(y0 + h), WHITE);

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

void signal_add(float s1[], float s2[], float out[],  int n){
    for(size_t i = 0; i < n; i++)
        out[i] = s1[i]+s2[i];
}

void signal_zero(float x[], size_t n) {
    for (size_t i = 0; i < n; i++) x[i] = 0.0f;
}

void signal_copy(const float in[], float out[], size_t n) {
    //TODO should I change this to memycopy?
    for (size_t i = 0; i < n; i++) out[i] = in[i];
}

void signal_scale(float x[], size_t n, float a) {
    for (size_t i = 0; i < n; i++) x[i] *= a;
}

float signal_sum(float x[], int n) {
    float sum = 0;
    for (size_t i = 0; i < n; i++) sum+= x[i];
    return sum;
}

void signal_mult(float x[], float y[], float out[], int n) {
    for (size_t i = 0; i < n; i++) out[i] = x[i] *  y[i];
}

void signal_accum(float out[], const float in[], size_t n, float w)
{
    for (size_t i = 0; i < n; i++) out[i] += w * in[i];
}

float signal_max_abs(const float x[], size_t n)
{
    float m = fabsf(x[0]);
    for (size_t i = 1; i < n; i++) {
        float a = fabsf(x[i]);
        if (a > m) m = a;
    }
    return m;
}

void signal_normalize(float x[], size_t n)
{
    float m = signal_max_abs(x, n);
    if (m <= 0.0f) return;
    for (size_t i = 0; i < n; i++) x[i] /= m;
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


void draw_spectrum_labels(
        float fs,
        size_t n,
        float x0, float y0, float w, float h,
        size_t every,
        Color col)
{
    if (n == 0 || every <= 0) return;

    float barW = w / (float)n;
    int fontSize = 12;

    for (size_t k = 0; k < n; k += every) {
        float x = x0 + k * barW;

        float freq = (fs * (float)k) / (float)n;

        DrawText(
                TextFormat("%i",(int)freq),
                (int)(x + 2),
                (int)(y0 + h + 4),
                fontSize,
                col
                );
    }
}


typedef struct{
    float *data;
    int size;
    int write_index;
} RingBuffer;


RingBuffer rb_init(int size){
    float *data = malloc(sizeof(float) * size);
    RingBuffer rg = {
        .data = data,
        .size = size,
        .write_index = 0
    }; 

    return rg;
}
void rb_write(RingBuffer* rg, float data){
    rg->data[rg->write_index % rg->size] = data;
    rg->write_index+=1;
}

float rb_read(const RingBuffer* rg, int index){
    if(rg->write_index < rg->size) return 0.0;
    int idx = (rg->write_index - rg->size + index) % rg->size;
    if (idx < 0) idx += rg->size;
    return rg->data[idx];
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

Color inferno(float t)
{
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

const float minZoom = 1.0f;
const float maxZoom = 5.0f;

void draw_spectrogram_zoom(Texture2D spectrogram_texture, int start_x, int start_y)
{
    float zoom_width = sw / 2 * zoom;
    float zoom_height = sh / 2 * zoom;

    Rectangle sourceRec = {0, 0, SPEC_W, N / 2};

    Rectangle destRec = {
        start_x - zoom_width / 2,    // Center the texture on the screen
        start_y - zoom_height / 2,
        zoom_width,                  // Apply zoom to width
        zoom_height                  // Apply zoom to height
    };

    Vector2 origin = {0, 0}; // Origin point for the texture (top-left)

    DrawTexturePro(spectrogram_texture, sourceRec, destRec, origin, 0.0f, WHITE);
}

void update_zoom() {
    // Keyboard zooming
    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
        zoom += 0.1f;
    }
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
        zoom -= 0.1f;
    }

    // Mouse scroll zooming
    float scroll = GetMouseWheelMove();
    if (scroll > 0) {
        zoom += 0.1f;  // Zoom in
    } else if (scroll < 0) {
        zoom -= 0.1f;  // Zoom out
    }

    // Keep zoom within limits
    if (zoom < minZoom) zoom = minZoom;
    if (zoom > maxZoom) zoom = maxZoom;
}

typedef struct {
    char *data;
    const char *format;
    int byte_size;
} FileMusic;

void draw_scene(const FileMusic *file){

        InitWindow(sw, sh, "Fourier Transform");
        InitAudioDevice();

        Music m = LoadMusicStreamFromMemory(file->format, file->data, file->byte_size); // Load music stream from data

        RenderTexture2D spectogram_texture = LoadRenderTexture(SPEC_W, N/2);
        float windows[N];
        float freq[N];
        m.looping = false; printf("Base audio info\n-sample rate: %d,\n-sample size: %d\n", m.stream.sampleRate, m.stream.sampleSize);
        total_samples = m.frameCount;
        samples = rb_init(N);
        printf("total samples%d\n", total_samples);
        freq_complex = malloc(sizeof(float complex) * N);

        SetTargetFPS(60);
        PlayMusicStream(m);

        AttachAudioStreamProcessor(m.stream, audio_callback);

        while (!WindowShouldClose())
        {
            UpdateMusicStream(m);
            BeginDrawing();
            ClearBackground(BLACK);
            update_zoom();

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
            spec_x = (spec_x + 1 ) % SPEC_W;
            draw_spectogram(sw/2, sh/2);

            BeginTextureMode(spectogram_texture);
            draw_spectrogram_zoom(spectogram_texture.texture, sw/2, sh/2);
            for (int y = 0; y < N/2; y++) {
                float v = spectrogram[spec_x][y];
                v = powf(v, 0.4f);
                Color c = inferno(v);
                DrawPixel(SPEC_W-1, (N/2 - y), c);
            }
            EndTextureMode();

            EndDrawing();
        }

        CloseAudioDevice();
        CloseWindow();
    }

long int find_size(const FILE *fp) 
{
    // checking if the file exist or not
    if (fp == NULL) {
        printf("File Not Found!\n");
        return -1;
    }

    fseek(fp, 0L, SEEK_END);

    // calculating the size of the file
    long int res = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    return res;
}

int main(){ 
    //const char* audio_path = "./audio/arctic.mp3";
    FILE *fptr;
    const char* audio_path = "./audio/bass.wav";

    fptr = fopen(audio_path, "rb");

    long int f_size = find_size(fptr); 
    if (f_size < 0){
        return 0;
    }
    char *buffer = malloc(sizeof(char) * f_size);
    printf("SIZE of file %ld", f_size);
    fread(buffer, f_size, 1, fptr);

    fclose(fptr);

    FileMusic fm = (FileMusic){
        .data = buffer,
        .byte_size = f_size,
        .format = ".wav"
    };
    
    draw_scene(&fm);
    return 0;
}
