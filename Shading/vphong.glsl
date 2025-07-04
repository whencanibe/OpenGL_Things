#version 330

in vec4 vPosition;
in vec4 vColor;
in vec4 vNormal;

out vec4 color;
out vec4 Pos;
out vec3 Normal;

uniform mat4 uProjMat;
uniform mat4 uModelMat;

void main(){

	gl_Position  = uProjMat * (uModelMat * vPosition);
	gl_Position *= vec4(1,1,-1,1);

	Pos = uModelMat*vPosition;

	vec4 normalVec = uModelMat * vec4(vNormal.xyz,0);
	Normal = normalize(normalVec.xyz);
	
	color = vColor;
}
