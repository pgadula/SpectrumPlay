
#ifndef RB_H
#define RB_H

typedef struct{
    float *data;
    int size;
    int write_index;
    int last_read_index;
} RingBuffer;

RingBuffer rb_init(int size);
void rb_write(RingBuffer* rb, float data);
float rb_read(const RingBuffer* rb, int index);
void rb_read_window(RingBuffer* rb, int window_size, float *output);
#endif

#ifdef RB_IMPLEMENTATION

RingBuffer rb_init(int size){
    float *data = malloc(sizeof(float) * size);
    RingBuffer rb = {
        .data = data,
        .size = size,
        .write_index = 0,
    }; 

    return rb;
}
void rb_write(RingBuffer* rb, float data){
    rb->data[rb->write_index % rb->size] = data;
    rb->write_index+=1;
}

float rb_read(const RingBuffer* rb, int index){
    if(rb->write_index < rb->size) return 0.0;
    int idx = (rb->write_index - rb->size + index) % rb->size;
    if (idx < 0) idx += rb->size;
    return rb->data[idx];
}

// ###
//   ###
//     ###
//       ###
//
// 
// w = 3  
//         idx
//         0-3
//         2-5
//         4-7
// start = 0 end = 3
// start = end - (1/4 * window) end = window + (end - (1/4 * window))

void rb_read_window(RingBuffer* rb, int window_size, float *output){
    int hop_size = 0.2 * window_size;

    int end = rb->last_read_index + hop_size;
    int start = end - window_size;

    for (int j = 0; j < window_size; j++){
        int i = start + j;
        output[j] = rb->data[i % rb->size];
    }
    rb->last_read_index = end;
}
#endif
