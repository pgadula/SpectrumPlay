#version 330

in vec2 fragTexCoord;

out vec4 finalColor;
uniform sampler2D spectogramTex;

struct Stop {
    float t;
    float r, g, b;
};
const Stop plasma[7] = Stop[](
    Stop(0.0, 0.050, 0.030, 0.530),
    Stop(0.15, 0.290, 0.070, 0.710),
    Stop(0.30, 0.510, 0.090, 0.650),
    Stop(0.50, 0.740, 0.150, 0.500),
    Stop(0.70, 0.900, 0.300, 0.300),
    Stop(0.85, 0.980, 0.550, 0.150),
    Stop(1.0, 0.940, 0.975, 0.130)
);

const Stop turbo[8] = Stop[](
    Stop(0.0, 0.190, 0.070, 0.230),
    Stop(0.14, 0.270, 0.290, 0.730),
    Stop(0.28, 0.180, 0.550, 0.910),
    Stop(0.42, 0.100, 0.750, 0.650),
    Stop(0.57, 0.450, 0.850, 0.250),
    Stop(0.72, 0.850, 0.820, 0.100),
    Stop(0.86, 0.950, 0.500, 0.080),
    Stop(1.0, 0.750, 0.080, 0.100)
);

const Stop magma[7] = Stop[](
    Stop(0.0, 0.000, 0.000, 0.000),
    Stop(0.15, 0.080, 0.020, 0.120),
    Stop(0.30, 0.250, 0.040, 0.300),
    Stop(0.50, 0.550, 0.090, 0.350),
    Stop(0.70, 0.850, 0.250, 0.200),
    Stop(0.85, 0.980, 0.550, 0.150),
    Stop(1.0, 0.990, 0.930, 0.750)
);

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
    Stop schema[] = magma;
    for(int i=0; i < 9; i++){
        if (v >= schema[i].t && v <= schema[i+1].t){
            //example1
            // [oo----v--c]
            //  ^     ^  ^
            //  5        10
            //size = c - o = 5
            // v = 8
            // from_start = v - c = 3;
            // from_start / size


            //example2
            // [oo----v--c]
            //  ^     ^  ^
            // 20     28 30
            //size = c - o = 10
            // v = 28
            // from_start = v - c = 8;
            // from_start / size
            
            float range = schema[i+1].t - schema[i].t;
            float t = (v - schema[i].t) / range; // 0..1

            vec3 c1 = vec3(schema[i].r, schema[i].g, schema[i].b);
            vec3 c2 = vec3(schema[i+1].r, schema[i+1].g, schema[i+1].b);

            //lerp
            vec3 final = mix(c1, c2, t); 
            return vec4(final, 1.0);
        }
    }
}

void main()
{
    vec4 g = texture(spectogramTex, fragTexCoord);
    vec4 c = get_color(g[0]);
    finalColor = vec4(c.r, c.g, c.b, 1.0);
}
