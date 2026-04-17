#version 330

in vec2 fragTexCoord;

out vec4 finalColor;
uniform sampler2D spectogramTex;

void main()
{
    vec4 g = texture(spectogramTex, fragTexCoord);
    finalColor = g;
}
