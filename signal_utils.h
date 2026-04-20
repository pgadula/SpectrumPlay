#ifndef SIGNAL_H
#define SIGNAL_H
#ifndef M_2PI 
#define M_2PI  M_PI*2
#endif

#include <complex.h>
#include <stdio.h>   
#include <math.h>   
#include <stddef.h> 

    void spectrum_abs_normalize(const float complex in[], float out[], size_t n);
    void signal_add(const float s1[], const float s2[], float out[], size_t n);
    void signal_zero(float x[], size_t n);
    void signal_copy(const float in[], float out[], size_t n);
    void signal_scale(float x[], size_t n, float a);
    float signal_sum(const float x[], size_t n);
    void signal_mult(const float x[], const float y[], float out[], size_t n);
    void signal_accum(float out[], const float in[], size_t n, float w);
    float signal_max_abs(const float x[], size_t n);
    void signal_normalize(float x[], size_t n);
    void log_spectrum(const float in[], float out[], size_t n, size_t out_n);
    void abs_spectrum(const float complex spec[], float out[], size_t n);
    void print_signal(const float in[], size_t n);
    void print_freq(const float complex freq[], size_t n);
    void gen_signal(float out[], double (*func)(double t), int f, float p, size_t n);
    void gen_signalI(float complex out[], float complex (*func)(float complex t), int f, float p, size_t n);
#endif

#ifdef SIGNAL_IMPLEMENTATION


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
void spectrum_abs_normalize(const complex float in[], float out[], size_t n) {
    float maxAbs = 0.0f;

    for (size_t i = 0; i < n; i++) {
        float a = cabsf(in[i]);
        if (a > maxAbs) maxAbs = a;
    }

    if (maxAbs <= 0.0f) {
        for (size_t i = 0; i < n; i++) out[i] = 0.0f;
        return;
    }

    for (size_t i = 0; i < n; i++) {
        out[i] = cabsf(in[i]) / maxAbs;
    }
}

void signal_add(const float s1[], const float s2[], float out[],  size_t n){
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

float signal_sum(const float x[], size_t n) {
    float sum = 0;
    for (size_t i = 0; i < n; i++) sum+= x[i];
    return sum;
}

void signal_mult(const float x[], const float y[], float out[], size_t n) {
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

void log_spectrum(const float in[], float out[], size_t n, size_t out_n)
{
    for (int i = 0; i < out_n; i++) out[i] = 0.0f;

    for (int i = 0; i < n; i++) {
        size_t bin = (int)(log2f(i + 1));
        if (bin >= out_n) bin = out_n - 1;

        if (in[i] > out[bin])
            out[bin] = in[i];
    }
}

void abs_spectrum(const float complex spec[],
                  float out[],
                  size_t n)
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

void print_freq(const float complex freq[], size_t n){
    printf("\n");
    for(size_t f = 0; f < n; f++){
       printf("[%zu]\t r:%f i:%f\n", f, crealf(freq[f]), cimagf(freq[f]));
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
#endif 
