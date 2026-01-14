#include <stdio.h>
#include <math.h>
#define N 16
#define F 2
#define M_2PI  M_PI*2

void fft(float in[], float out [], size_t n)
void print(float in[], size_t n)
void gen_signal(float in[], float out[], size_t n)

int main(){
    float signal[N+1];

    for(size_t i=0; i<=N; i++){
        signal[i]=0;
    }

    for(size_t i = 0; i <= N; i++){
        float t = (float)i/(float)N;
        float v1 = cos(t*M_2PI*F);
        float v2 = sin(t*M_2PI*F);

        signal[i]=v1*v2;
    }

    print(signal, N);

    return 0;
}

void fft(float in[], float out [], size_t n){
    return;
}

void print(float in[], size_t n){
    printf("\n");
    for(size_t i = 0; i <= N; i++){
       printf("%f\n", in[i]);
    }
}

void gen_signal(float in[], float out[], size_t n){
    for(size_t i = 0; i <= N; i++){
       float t = (float)i/(float)N;
       in[i] = sin(t*M_2PI*F);
    }
}
