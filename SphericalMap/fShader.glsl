#version 330
in vec2 T2;

out vec4 fColor;

uniform float uTime;
uniform sampler2D uMapTex;

void main()
{
	vec4 texColor = texture(uMapTex, T2);

    fColor = texColor;
	fColor.w = 1;
}