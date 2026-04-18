
#ifndef RB_H
#define RB_H

typedef struct{
    float *data;
    int size;
    int write_index;
} RingBuffer;

RingBuffer rb_init(int size);
void rb_write(RingBuffer* rg, float data);
float rb_read(const RingBuffer* rg, int index);

#endif

#ifdef RB_IMPLEMENTATION

RingBuffer rb_init(int size){
    float *data = malloc(sizeof(float) * size);
    RingBuffer rg = {
        .data = data,
        .size = size,
        .write_index = 0
    }; 

    return rg;
}
void rb_write(RingBuffer* rg, float data){
    rg->data[rg->write_index % rg->size] = data;
    rg->write_index+=1;
}

float rb_read(const RingBuffer* rg, int index){
    if(rg->write_index < rg->size) return 0.0;
    int idx = (rg->write_index - rg->size + index) % rg->size;
    if (idx < 0) idx += rg->size;
    return rg->data[idx];
}
#endif
