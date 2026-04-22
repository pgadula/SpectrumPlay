#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform float freq[512];
const int BARS = 512;
const int BIN_PER_BAR = BARS/16;

struct Stop {
    float t;
    float r, g, b;
};
const Stop inferno[8] = Stop[](
    Stop(0.0, 0.001, 0.000, 0.013),
    Stop(0.15, 0.120, 0.030, 0.200),
    Stop(0.3, 0.350, 0.040, 0.300),
    Stop(0.45, 0.600, 0.070, 0.200),
    Stop(0.6, 0.850, 0.200, 0.050),
    Stop(0.75, 0.980, 0.500, 0.100),
    Stop(0.9, 0.990, 0.800, 0.300),
    Stop(1.0, 0.990, 0.990, 0.750)
);

vec4 get_color(float v){
    Stop schema[] = inferno;
    for(int i=0; i < 9; i++){
        if (v >= schema[i].t && v <= schema[i+1].t){
            float t = (v - schema[i].t) / (schema[i+1].t - schema[i].t);
            vec3 final = mix(vec3(schema[i].r, schema[i].g, schema[i].b), vec3(schema[i+1].r, schema[i+1].g, schema[i+1].b), t);
            return vec4(final, 1.0);
        }
    }
}

void main(){
    float x = fragTexCoord.x;
    float y = fragTexCoord.y;
    int idx = int(floor(x * BARS));

    int start = int(floor((idx / BIN_PER_BAR)) * BIN_PER_BAR);
    int end = start + BIN_PER_BAR; 
    float t = 0;
    for(int i = start; i < end; i++){
        t = max(t, freq[i]);
    }

    vec4 color = get_color(t);
    if(y > t)
        finalColor = vec4(0.0);
    else{
        finalColor = color; 
    }
}
