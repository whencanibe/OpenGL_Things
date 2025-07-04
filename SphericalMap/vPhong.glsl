#version 330

in  vec4 vPosition;
in  vec4 vColor;
in  vec3 vNormal; 
in  vec2 vTexCoord;

out vec3 N3; 
out vec3 L3; 
out vec3 V3;  
out vec2 T2;

out vec3 wV;
out vec3 wP;
out vec3 wN;

uniform mat4 uModelMat; 
uniform mat4 uProjMat; 
uniform vec4 uLPos; 

uniform vec4 uEPos;


void main()
{
	gl_Position  = uProjMat*uModelMat*vPosition;
	gl_Position *= vec4(1,1,-1,1);
   
	vec4 N = uModelMat*vec4(vNormal,0); 
	vec4 L = uLPos - uModelMat*vPosition; 
	vec4 V = vec4(0, 0, 0, 1) - uModelMat*vPosition; 

	N3 = normalize(N.xyz); 
	L3 = normalize(L.xyz); 
	V3 = normalize(V.xyz); 
	
	T2 = vTexCoord;
	T2.x = 1.0 - T2.x;

	wP = vPosition.xyz;
	wN = vec4(vNormal,0).xyz;
	wV = uEPos.xyz;
}
