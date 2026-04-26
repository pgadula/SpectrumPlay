#version 100

precision mediump float;

varying vec2 fragTexCoord;

uniform float window[1024];

void main()
{
    int idx = int(fragTexCoord.x * 1023.0);

    if (idx < 0) idx = 0;
    if (idx > 1023) idx = 1023;

    float v = 0.0;

    for (int i = 0; i < 1024; i++) {
        if (i == idx) {
            v = window[i];
        }
    }

    float sampleY = v * 0.5 + 0.5;
    float d = abs(sampleY - fragTexCoord.y);
    float line = smoothstep(0.01, 0.0, d);

    gl_FragColor = vec4(line, line, line, 1.0);
}
