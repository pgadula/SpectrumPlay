#ifndef TEST_SIGNAL_H
#define TEST_SIGNAL_H

#include "ring_buffer.h"

void test_impulse();
void test_dc();
void test_sine_bin();
void sweapy();

#endif
#ifdef TEST_SIGNAL_IMPLEMENTATION
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
#endif 
