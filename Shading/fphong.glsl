#version 330

in vec4 color;
in vec4 Pos;
in vec3 Normal;

out vec4 fColor;

uniform mat4 uProjMat;
uniform mat4 uModelMat;

uniform vec4 ka;
uniform vec4 kd;
uniform vec4 ks;
uniform vec4 I;
uniform vec4 lightPos;
uniform float shine;
uniform vec4 cam;

void main()
{
	vec3 normal3 = normalize(Normal);

    vec4 ObjToLight = lightPos - Pos;
	vec3 ObjToLight3 = normalize(ObjToLight.xyz);

	vec4 diffuse = kd * I * max(dot(ObjToLight3,normal3),0);

	vec4 amb = ka * I;
	
	vec4 ObjToCam = cam - Pos;
	vec3 ObjToCam3 = normalize(ObjToCam.xyz);
	vec3 ref3 = 2*max(dot(normal3,ObjToLight3),0) * normal3 - ObjToLight3;

	vec4 spec = ks * I * pow(max(dot(ref3, ObjToCam3),0),shine);
	
	fColor =  diffuse + amb + spec;
}

