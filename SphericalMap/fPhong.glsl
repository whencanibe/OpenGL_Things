#version 330
in vec3 N3; 
in vec3 L3; 
in vec3 V3; 
in vec2 T2;

in vec3 wV;
in vec3 wP;
in vec3 wN;

out vec4 fColor;

uniform mat4 uModelMat; 
uniform mat4 uProjMat; 
uniform vec4 uLPos; 
uniform vec4 uAmb; 
uniform vec4 uDif; 
uniform vec4 uSpc; 

uniform sampler2D uTex;
uniform sampler2D uMapTex;


uniform float uTime;
uniform float uFresnel;
uniform bool uIsDiffMap;

void main()
{
	vec3 N = normalize(N3); 
	vec3 L = normalize(L3); 
	vec3 V = normalize(V3); 
	vec3 H = normalize(V+L); 

    float NL = max(dot(N, L), 0); 
	
	vec4 texColor = texture(uTex, T2);

	vec4 phongColor = uAmb + uDif*NL;

	vec3 viewDir = wP - wV;

	viewDir = normalize(viewDir);
	vec3 R = reflect(-viewDir, wN);
	R = normalize(R);
	
	float u = 0.5 + atan(R.z, R.x) / (2.0 * 3.141592);
    float v = 0.5 - asin(R.y) / 3.141592;

    vec4 reflectColor = texture(uMapTex, vec2(u, v));

    vec4 diffuseMapColor = texColor;

    // Fresnel
    float F = 0.0;
    float ratio = F + (1.0 - F) * pow(1.0 + dot(viewDir, wN), uFresnel);
    if (uFresnel > 10.0) ratio = 0.0;
	
	fColor = mix(phongColor, reflectColor, ratio);

	if(uIsDiffMap)
		fColor = mix(diffuseMapColor, reflectColor, ratio);
    fColor.w = 1.0; 
}
