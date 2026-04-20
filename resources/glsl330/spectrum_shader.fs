#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform float freq[512];

void main()
{
    const int BARS = 64;
    const int BIN_PER_BAR = 8;

    int bar = clamp(int(fragTexCoord.x * float(BARS)), 0, BARS - 1);

    float v = 0.0;
    for (int i = 0; i < BIN_PER_BAR; i++) {
        int idx = bar * BIN_PER_BAR + i;
        v = max(v, freq[idx]);
    }

    v = clamp(v, 0.0, 1.0);

    float y = 1.0 - fragTexCoord.y;

    float fill = step(y, v);
    finalColor = vec4(fill, fill, fill, 1.0);
}
