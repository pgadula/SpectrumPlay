#ifdef FFT_IMPLEMENTATION
#define FFT_IMPLEMENTATION

#include <complex.h>
#include <math.h>
#define M_2PI  M_PI*2

void fft(const float in[], float complex out[], size_t stride,  size_t n){
    if(n == 1){
        out[0] = in[0];
        return;
    }

    // Split the input signal into even- and odd-indexed samples and recurse.
    // o o o o o o o o
    // o x o x o x o x
    // q x o r q x o r
    // o = (2, 6) x = (1, 5) q = (0, 4) r = (3, 7)

    //even
    fft(in, out, stride * 2, n/2);
    //odd
    fft(in + stride, out + n / 2, stride * 2, n/2);

    for(size_t k = 0; k < n/2; k++){
        float c = (float)k / (float)n;
        float angle = -2 * M_PI * c;

        float complex tw = cexpf(angle * I);

        //even
        float complex e  = out[k];
        //odd
        float complex o  = out[k + n / 2];


         //t = exp(-j*2π*k/N) * O[k]
         float complex t = tw * o;
         
         out[k]       =  e + t;
         out[k + n / 2] =  e - t;
    }
}

void dft(float in[], float complex out[], size_t n){
    for(size_t f = 0; f < n; f++){
        float complex freq_signal[n];
        float ff = M_2PI * f;
        out[f] = 0;
        for(size_t i = 0; i < n; i++){
            float t = (float)i/(float)(n-1);
            freq_signal[i] = cexpf(-I * t * ff);
            out[f] += freq_signal[i] * in[i];
        }
    }
}

#endif FFT_IMPLEMENTATION
