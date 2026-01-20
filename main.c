#include <complex.h>
#include <stdio.h>
#include "raylib.h"
#include <math.h>
#define N 32 
#define M_2PI  M_PI*2

const size_t sw = 1680;
const size_t sh = 1024;
const size_t half_sh = sh/2;
const size_t r = 5;

void print(float in[], size_t n){
    printf("\n");
    for(size_t i = 0; i < n; i++){
       printf("%f\n", in[i]);
    }
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

void gen_signal(float output[], double (*func)(double t), int f, float p, size_t n){
    float ff = M_2PI * f;
    for(size_t i = 0; i < n; i++){
        float t = (float)i/(float)(n-1);
        float v = (*func)(t * ff + p);
        output[i] = v;
    }
}

void gen_signalI(float complex output[], float complex (*func)(float complex t), int f, float p, size_t n){
    float ff = M_2PI * f;
    for(size_t i = 0; i < n; i++){
        float t = (float)i/(float)(n-1);
        float v = (*func)(t * ff + p);
        output[i] = v;
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

        DrawLine((int)x1, (int)y1, (int)x2, (int)y2, col);
    }
}

void spectrum_abs_normalize(const float in[], float out[], size_t n)
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

    DrawLine((int)x0, (int)(y0 + h), (int)(x0 + w), (int)(y0 + h), WHITE);

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



void signal_add(float s1[], float s2[], float output[],  int n){
    for(size_t i = 0; i < n; i++)
        output[i] = s1[i]+s2[i];
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

void signal_mult(float x[], float y[], float output[], int n) {
    for (size_t i = 0; i < n; i++) output[i] = x[i] *  y[i];
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


void dft(float in[], float complex output[], size_t n){
    for(size_t f = 0; f < n; f++){
        float complex freq_signal[n];
        float ff = M_2PI * f;
        output[f] = 0;
        for(size_t i = 0; i < n; i++){
           
            float t = (float)i/(float)(n-1);
            freq_signal[i] = cexpf(t * ff * I);
            output[f] += freq_signal[i] * in[i];
        }
    }
}

void dft_slow(float in[], float freq[], size_t n){
    float r[n];
    signal_copy(in, r, n);
    for(size_t f = 0; f < n; f++){

        float freq_signal[n];
        gen_signal(freq_signal, sin, f, 0, N);

        float o[n];
        signal_mult(r, freq_signal, o, n);

        freq[f] = signal_sum(o, n);
    }
    print(freq, n);
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

int main(){
    float s1[N];
    float s2[N];
    float s3[N];

    float complex freq[N];

    gen_signal(s1, sin, 3, 0, N);
    gen_signal(s2, cos, 9, 0, N);
    signal_add(s1, s2, s3,  N);
    dft(s3, freq, N);
  //  spectrum_abs_normalize(freq, freq, N);
  //  
  //  signal_normalize(s1, N);
  //  InitWindow(sw, sh, "dft_slow");
  //  SetTargetFPS(60);

  //  while (!WindowShouldClose())
  //  {
  //      BeginDrawing();
  //      ClearBackground(BLACK);

  //      draw_signal_norm_area(s3, N, sw/2+10, 120, sw/2-10, sh/5, RED);

  //      draw_spectrum_rects(freq, N,   10, 150, sw/2-10, sh/3, RED, GREEN);
  //      draw_spectrum_labels(N, N,   10, 150, sw/2-10, sh/3, 1, LIGHTGRAY);

  //      DrawLine(0, half_sh, sw, half_sh, WHITE);

  //      EndDrawing();
  //  }

  //  CloseWindow();
    print_freq(freq, N);

    return 0;
}
