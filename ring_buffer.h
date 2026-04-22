
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
#endif
