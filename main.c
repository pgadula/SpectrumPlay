#include <stdio.h>
#include "raylib.h"
#include <math.h>
#define N 128
#define F 2
#define M_2PI  M_PI*2

const int sw = 1680;
const int sh = 1024;
const int half_sh = sh/2;
float cw = sw/(N-1);
const int r = 5;

void fft(float in[], float out [], size_t n){
    return;
}

void print(float in[], size_t n){
    printf("\n");
    for(size_t i = 0; i <= N; i++){
       printf("%f\n", in[i]);
    }
}

void gen_signal(float in[], size_t n){
    for(size_t i = 0; i <= N; i++){
        float v1, v2;
        float t = (float)i/(float)N;
        v1 = sin(t*M_2PI*F);
        v2 = sin(t*M_2PI*F);
        float v = v1 + v2;
        in[i] = v;
    }
}

void get_min_max(float in[], size_t n, float *min, float *max){
    float c_min = 999999;
    float c_max = -999999;
    for(size_t i = 0; i <= N; i++){
        float v = in[i];
        if(c_min > v){
            c_min = v;
        }
        if(c_max < v){
            c_max = v;
        }
    }
    printf("min:%f, max:%f\n", c_min, c_max);
    (*min) = c_min;
    (*max) = c_max;
}

void gen_points(float in[], Vector2 points[], size_t n, float max, float min){
    for(size_t i = 0; i <= N; i++){
        float p = in[i] / max;
        points[i] = (Vector2){ .x = cw*i+r, .y = half_sh + (p*(half_sh-r))};
    }
}

int main(){
    float signal[N+1];
    float max, min;
    Vector2 points[N+1];

    gen_signal(signal, N);
    get_min_max(signal, N, &min, &max);

    gen_points(signal, points, N, max, min);

    InitWindow(sw, sh, "FFT");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawLine(0, half_sh, sw, half_sh, WHITE);
        for(size_t i = 0; i <= N-1; i++){
            Vector2 p1 = points[i];
            Vector2 p2 = points[i+1];
            DrawLineV(p1, p2, RED);
        }

        for(size_t i = 0; i <= N; i++){
            Vector2 p = points[i];
            DrawCircleV(p, r, GREEN);
        }
        EndDrawing();
    }

    CloseWindow();

    print(signal, N);
    //printf("max:%f, min:%f", *min, *max);
    return 0;
}
