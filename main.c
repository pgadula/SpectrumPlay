#include <assert.h>
#include <complex.h>
#include <stdio.h>
#include "raylib.h"
#include <math.h>
#define FFT_IMPLEMENTATION
#include "fft.h"
#define N 512 
#define LOG_N 32 
#define M_2PI  M_PI*2

const size_t sw = 1680;
const size_t sh = 1024;
const size_t half_sh = sh/2;
const size_t r = 5;

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
    float max = 0.0f;

    for (int i = 0; i < n; i++) {
        float v = 20.0f * log10f(cabsf(spec[i]) + 1e-6f);
        out[i] = v; 
        if (out[i] > max)
            max = out[i];
    }

    if (max > 0.0f) {
        for (int i = 0; i < n; i++) 
            out[i] /= max;
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


float current_frame[N];
int current_size;

void a_callback(void *bufferData, unsigned int frames){
    current_size = frames;
    for(int frame = 0; frame < N; frame++) current_frame[frame] = 0;

    float *buffer = (float *)bufferData;

    for(int frame = 0; frame < frames; frame++){
        float multi = 0.5 * ( 1 - cos( (2 * PI * frame) / frames));
        current_frame[frame] = buffer[frame * 2 + 0] * multi; //take only left channel
    }
}

int main(){
    float complex freq_complex[N];
    float freq[N];
    InitWindow(sw, sh, "Fourier Transform");
    InitAudioDevice();
    const char* audio_path = "./audio/arctic.mp3";
   // const char* audio_path = "./audio/bass.wav";
    Music m = LoadMusicStream(audio_path);
    printf("Base audio info\n-sample rate: %d,\n-sample size: %d\n", m.stream.sampleRate, m.stream.sampleSize);
    SetTargetFPS(60);
    PlayMusicStream(m);

   AttachAudioStreamProcessor(m.stream, a_callback);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(m);
        BeginDrawing();
        ClearBackground(BLACK);

        fft(current_frame, freq_complex, 1, N);
        abs_spectrum(freq_complex, freq, N);

        draw_signal_norm_area(current_frame, current_size, 0, sh/2, sw/2, sh/2, RED);
        draw_spectrum_rects(freq, current_size/2, 5, 160, sw, sh/3, BLUE, BLUE);
        //DrawLine(0, half_sh, sw, half_sh, WHITE);

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
