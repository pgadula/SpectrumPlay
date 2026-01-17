#include <stdio.h>
#include "raylib.h"
#include <math.h>
#define N 128
#define M_2PI  M_PI*2

const size_t sw = 1680;
const size_t sh = 1024;
const size_t half_sh = sh/2;
const size_t r = 5;

float cw = sw/(N-1);

void fft(float in[], float out [], size_t n){
    return;
}

void print(float in[], size_t n){
    printf("\n");
    for(size_t i = 0; i < n; i++){
       printf("%f\n", in[i]);
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
        points[i] = (Vector2){ .x = cw*i+r, .y = half_sh + (p*(half_sh-r))};
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

void signal_add(float s1[], float s2[], float output[],  int n){
    for(size_t i = 0; i < n; i++)
        output[i] += s1[i]+s2[i];
}

void signal_zero(float x[], size_t n) {
    for (size_t i = 0; i < n; i++) x[i] = 0.0f;
}

void signal_copy(const float in[], float out[], size_t n) {
    for (size_t i = 0; i < n; i++) out[i] = in[i];
}

void signal_scale(float x[], size_t n, float a) {
    for (size_t i = 0; i < n; i++) x[i] *= a;
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

int main(){
    float max, min;
    float s1[N];
    float s2[N];
    float s3[N];

    gen_signal(s1, sin, 2, 0, N);
    gen_signal(s2, cos, 9, 0, N);
    gen_signal(s3, sin, 9, 0, N);
    signal_accum(s3, s1, N, 1);
    signal_accum(s3, s2, N, 1);
     
    
    InitWindow(sw, sh, "FFT");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        signal_normalize(s2, N);
        draw_signal_norm_area(s2, N, sw/2, sh/2, sw/2, sh/5, RED);

        signal_normalize(s3, N);
        draw_signal_norm_area(s3, N, 0, 200, sw/2, sh/2, GREEN);

        DrawLine(0, half_sh, sw, half_sh, WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
