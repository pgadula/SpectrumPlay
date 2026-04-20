#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform float window[1024];

void main()
{
    int idx = clamp(int(fragTexCoord.x * 1023.0), 0, 1023);

    float v = window[idx];   
    float sampleY = v * 0.5 + 0.5;
    float d = abs(sampleY - fragTexCoord.y);
    float line = smoothstep(0.01, 0.0, d);

    finalColor = vec4(line, line, line, 1.0);
}
