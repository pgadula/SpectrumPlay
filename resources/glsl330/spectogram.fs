#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D spectogramTex;
uniform int color_schema;

const int MAX_STOPS = 8;

struct Stop {
    float t;
    float r, g, b;
};

const Stop plasma[MAX_STOPS] = Stop[](
    Stop(0.0, 0.050, 0.030, 0.530),
    Stop(0.15, 0.290, 0.070, 0.710),
    Stop(0.30, 0.510, 0.090, 0.650),
    Stop(0.50, 0.740, 0.150, 0.500),
    Stop(0.70, 0.900, 0.300, 0.300),
    Stop(0.85, 0.980, 0.550, 0.150),
    Stop(1.0, 0.940, 0.975, 0.130),
    Stop(1.0, 0.940, 0.975, 0.130)
);

const Stop turbo[MAX_STOPS] = Stop[](
    Stop(0.0, 0.190, 0.070, 0.230),
    Stop(0.14, 0.270, 0.290, 0.730),
    Stop(0.28, 0.180, 0.550, 0.910),
    Stop(0.42, 0.100, 0.750, 0.650),
    Stop(0.57, 0.450, 0.850, 0.250),
    Stop(0.72, 0.850, 0.820, 0.100),
    Stop(0.86, 0.950, 0.500, 0.080),
    Stop(1.0, 0.750, 0.080, 0.100)
);

const Stop magma[MAX_STOPS] = Stop[](
    Stop(0.0, 0.000, 0.000, 0.000),
    Stop(0.15, 0.080, 0.020, 0.120),
    Stop(0.30, 0.250, 0.040, 0.300),
    Stop(0.50, 0.550, 0.090, 0.350),
    Stop(0.70, 0.850, 0.250, 0.200),
    Stop(0.85, 0.980, 0.550, 0.150),
    Stop(1.0, 0.990, 0.930, 0.750),
    Stop(1.0, 0.990, 0.930, 0.750)
);

const Stop inferno[MAX_STOPS] = Stop[](
    Stop(0.0, 0.001, 0.000, 0.013),
    Stop(0.15, 0.120, 0.030, 0.200),
    Stop(0.3, 0.350, 0.040, 0.300),
    Stop(0.45, 0.600, 0.070, 0.200),
    Stop(0.6, 0.850, 0.200, 0.050),
    Stop(0.75, 0.980, 0.500, 0.100),
    Stop(0.9, 0.990, 0.800, 0.300),
    Stop(1.0, 0.990, 0.990, 0.750)
);

vec4 sampleGradient(const Stop schema[MAX_STOPS], int n, float v) {
    v = clamp(v, 0.0, 1.0);

    for (int i = 0; i < n - 1; i++) {
        if (v >= schema[i].t && v <= schema[i + 1].t) {
            float range = schema[i + 1].t - schema[i].t;
            float t = (v - schema[i].t) / range;

            vec3 c1 = vec3(schema[i].r, schema[i].g, schema[i].b);
            vec3 c2 = vec3(schema[i + 1].r, schema[i + 1].g, schema[i + 1].b);

            return vec4(mix(c1, c2, t), 1.0);
        }
    }

    return vec4(schema[n - 1].r, schema[n - 1].g, schema[n - 1].b, 1.0);
}

vec4 sampleSelectedGradient(int scheme, float t) {
    if (scheme == 0) return sampleGradient(plasma, 7, t);
    if (scheme == 1) return sampleGradient(turbo, 8, t);
    if (scheme == 2) return sampleGradient(magma, 7, t);
    return sampleGradient(inferno, 8, t);
}

void main() {
    float v = texture(spectogramTex, fragTexCoord).r;
    finalColor = sampleSelectedGradient(color_schema, v);
}
