#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform sampler2D spectogramTex;
uniform int color_schema;

vec3 plasmaColor(float v) {
    v = clamp(v, 0.0, 1.0);

    if (v < 0.15) return mix(vec3(0.050, 0.030, 0.530), vec3(0.290, 0.070, 0.710), v / 0.15);
    else if (v < 0.30) return mix(vec3(0.290, 0.070, 0.710), vec3(0.510, 0.090, 0.650), (v - 0.15) / 0.15);
    else if (v < 0.50) return mix(vec3(0.510, 0.090, 0.650), vec3(0.740, 0.150, 0.500), (v - 0.30) / 0.20);
    else if (v < 0.70) return mix(vec3(0.740, 0.150, 0.500), vec3(0.900, 0.300, 0.300), (v - 0.50) / 0.20);
    else if (v < 0.85) return mix(vec3(0.900, 0.300, 0.300), vec3(0.980, 0.550, 0.150), (v - 0.70) / 0.15);
    else return mix(vec3(0.980, 0.550, 0.150), vec3(0.940, 0.975, 0.130), (v - 0.85) / 0.15);
}

vec3 turboColor(float v) {
    v = clamp(v, 0.0, 1.0);

    if (v < 0.14) return mix(vec3(0.190, 0.070, 0.230), vec3(0.270, 0.290, 0.730), v / 0.14);
    else if (v < 0.28) return mix(vec3(0.270, 0.290, 0.730), vec3(0.180, 0.550, 0.910), (v - 0.14) / 0.14);
    else if (v < 0.42) return mix(vec3(0.180, 0.550, 0.910), vec3(0.100, 0.750, 0.650), (v - 0.28) / 0.14);
    else if (v < 0.57) return mix(vec3(0.100, 0.750, 0.650), vec3(0.450, 0.850, 0.250), (v - 0.42) / 0.15);
    else if (v < 0.72) return mix(vec3(0.450, 0.850, 0.250), vec3(0.850, 0.820, 0.100), (v - 0.57) / 0.15);
    else if (v < 0.86) return mix(vec3(0.850, 0.820, 0.100), vec3(0.950, 0.500, 0.080), (v - 0.72) / 0.14);
    else return mix(vec3(0.950, 0.500, 0.080), vec3(0.750, 0.080, 0.100), (v - 0.86) / 0.14);
}

vec3 magmaColor(float v) {
    v = clamp(v, 0.0, 1.0);

    if (v < 0.15) return mix(vec3(0.000, 0.000, 0.000), vec3(0.080, 0.020, 0.120), v / 0.15);
    else if (v < 0.30) return mix(vec3(0.080, 0.020, 0.120), vec3(0.250, 0.040, 0.300), (v - 0.15) / 0.15);
    else if (v < 0.50) return mix(vec3(0.250, 0.040, 0.300), vec3(0.550, 0.090, 0.350), (v - 0.30) / 0.20);
    else if (v < 0.70) return mix(vec3(0.550, 0.090, 0.350), vec3(0.850, 0.250, 0.200), (v - 0.50) / 0.20);
    else if (v < 0.85) return mix(vec3(0.850, 0.250, 0.200), vec3(0.980, 0.550, 0.150), (v - 0.70) / 0.15);
    else return mix(vec3(0.980, 0.550, 0.150), vec3(0.990, 0.930, 0.750), (v - 0.85) / 0.15);
}

vec3 infernoColor(float v) {
    v = clamp(v, 0.0, 1.0);

    if (v < 0.15) return mix(vec3(0.001, 0.000, 0.013), vec3(0.120, 0.030, 0.200), v / 0.15);
    else if (v < 0.30) return mix(vec3(0.120, 0.030, 0.200), vec3(0.350, 0.040, 0.300), (v - 0.15) / 0.15);
    else if (v < 0.45) return mix(vec3(0.350, 0.040, 0.300), vec3(0.600, 0.070, 0.200), (v - 0.30) / 0.15);
    else if (v < 0.60) return mix(vec3(0.600, 0.070, 0.200), vec3(0.850, 0.200, 0.050), (v - 0.45) / 0.15);
    else if (v < 0.75) return mix(vec3(0.850, 0.200, 0.050), vec3(0.980, 0.500, 0.100), (v - 0.60) / 0.15);
    else if (v < 0.90) return mix(vec3(0.980, 0.500, 0.100), vec3(0.990, 0.800, 0.300), (v - 0.75) / 0.15);
    else return mix(vec3(0.990, 0.800, 0.300), vec3(0.990, 0.990, 0.750), (v - 0.90) / 0.10);
}

vec3 sampleSelectedGradient(int scheme, float t) {
    if (scheme == 0) return plasmaColor(t);
    if (scheme == 1) return turboColor(t);
    if (scheme == 2) return magmaColor(t);
    return infernoColor(t);
}

void main() {

    float v = texture2D(spectogramTex, fragTexCoord).r;
    vec3 color = sampleSelectedGradient(color_schema, v);
    gl_FragColor = vec4(color, 1.0);
}
