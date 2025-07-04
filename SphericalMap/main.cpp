#define _CRT_SECURE_NO_WARNINGS

#include <vgl.h>
#include <InitShader.h>
#include "MySphere.h"
#include "Targa.h"
#include "MyObject.h"

#include <vec.h>
#include <mat.h>
#include <cmath>

MySphere sphere;
MyObject bunny;

GLuint map_prog;
GLuint phong_prog;

float g_Time = 0;
float g_aspect = 1;

vec2 lastMousePos;
bool isDraggingLeft = false;
bool isDraggingRight = false;
float cameraDistance = 5.0f; 
float alpha = 0.0f;           
float beta = 0.0f;         

vec3 ePos = vec3(0, 0, 5);
int drawingMode = 0;
float fresnel = 10;
bool isDiffuseMap = false;

std::string groupName;

mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	mat4 V = mat4(1.0f);
	
	up = normalize(up);
	vec3 n = normalize(at - eye);
	float a = dot(up, n);
	vec3 v = normalize(up-a*n);
	vec3 w = cross(n, v);

	V[0] = vec4(w, dot(-w, eye));
	V[1] = vec4(v, dot(-v, eye));
	V[2] = vec4(-n, dot(n, eye));
	
	return V;
}

mat4 myOrtho(float l, float r, float b, float t, float zNear, float zFar)
{
	vec3 center = vec3((l+r)/2, (b+t)/2, -(zNear)/2);
	mat4 T = Translate(-center);
	mat4 S = Scale(2/(r-l), 2/(t-b), -1/(-zNear+zFar));
	mat4 V = S*T;

	return V;
}

mat4 myPerspective(float angle, float aspect, float zNear, float zFar)
{
	float rad = angle*3.141592/180.0f;
	mat4 V(1.0f);
	float h = 2*zFar*tan(rad/2);
	float w = aspect*h;
	mat4 S = Scale(2/w, 2/h, 1/zFar);

	float c = -zNear/zFar;
	
	mat4 Mpt(1.0f);
	Mpt[2] = vec4(0, 0, 1/(c+1), -c/(c+1));
	Mpt[3] = vec4(0, 0, -1, 0);
	
	V = Mpt*S;

	return V;

}

void textureInit(std::string groupName) {
	STGA image;

	std::string mapFile = groupName + "_spheremap.tga";
	std::string diffMapFile = groupName + "_diffusemap.tga";

	image.loadTGA(mapFile.c_str());
	int w = image.width;
	int h = image.height;
	printf("image size = %d, %d\n", w, h);

	GLuint tex;
	glGenTextures(1, &tex);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D,
		0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

	image.destroy();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	GLuint tex2;
	glGenTextures(1, &tex2);
	image.loadTGA(diffMapFile.c_str());
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex2);

	w = image.width;
	h = image.height;

	glTexImage2D(GL_TEXTURE_2D,
		0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

	image.destroy();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}


void myInit()
{
	sphere.Init(40,40);
	bunny.Init("bunny.obj");

	map_prog = InitShader("vShader.glsl", "fShader.glsl");
	glUseProgram(map_prog);

	phong_prog = InitShader("vPhong.glsl", "fPhong.glsl");
	glUseProgram(phong_prog);

	textureInit(groupName);
}

mat4 ModelMat = myLookAt(ePos, vec3(0, 0, 0), vec3(0, 1, 0));

void display()
{
	glClearColor(0,0,0,1);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	mat4 mapModelMat = myLookAt(ePos, vec3(0, 0, 0), vec3(0, 1, 0)) * Scale(10,10,10);
	
	mat4 ProjMat = myPerspective(45, g_aspect, 0.01, 100.0f);
	
	glUseProgram(map_prog);
	
	// 1. Define Light Properties
	// 
	vec4 lPos = vec4(20, 10, 20, 1);			 
	vec4 lAmb = vec4(0.3, 0.3, 0.3, 1);		
	vec4 lDif = vec4(0.5, 0.5, 0.5, 1);
	vec4 lSpc = lDif; 

	// 2. Define Material Properties
	//
	vec4 mAmb = vec4(0.3, 0.3, 0.3, 1);		
	vec4 mDif = vec4(1.0, 1.0, 1.0, 1);		
	vec4 mSpc = vec4(1.3, 1.3, 1.3, 1); 
	float mShiny = 100;										//1~100;

	// I = lAmb*mAmb + lDif*mDif*(N¡¤L) + lSpc*mSpc*(V¡¤R)^n; 
	vec4 amb = lAmb*mAmb;					
	vec4 dif = lDif*mDif;					
	vec4 spc = lSpc*mSpc; 

	// 3. Send Uniform Variables to the shader
	//

	GLuint uMat = glGetUniformLocation(map_prog, "uMat");
	GLuint uTime = glGetUniformLocation(map_prog, "uTime");
	GLuint uMapTex = glGetUniformLocation(map_prog, "uMapTex");

	glUniformMatrix4fv(uMat, 1, true, ProjMat*mapModelMat);
	glUniform1f(uTime, g_Time);
	glUniform1i(uMapTex, 0);

	sphere.Draw(map_prog);

	glUseProgram(phong_prog);

	GLuint uModelMat = glGetUniformLocation(phong_prog, "uModelMat");
	GLuint uProjMat = glGetUniformLocation(phong_prog, "uProjMat");
	GLuint uLPos = glGetUniformLocation(phong_prog, "uLPos");
	GLuint uAmb = glGetUniformLocation(phong_prog, "uAmb");
	GLuint uDif = glGetUniformLocation(phong_prog, "uDif");
	
	
	GLuint uEPos = glGetUniformLocation(phong_prog, "uEPos");
	GLuint uIsDiffMap = glGetUniformLocation(phong_prog, "uIsDiffMap");

	GLuint uTex = glGetUniformLocation(phong_prog, "uTex");
	uMapTex = glGetUniformLocation(phong_prog, "uMapTex");
	GLuint uFresnel = glGetUniformLocation(phong_prog, "uFresnel");

	glUniform1i(uIsDiffMap, isDiffuseMap);
	glUniform1f(uFresnel, fresnel);
	glUniformMatrix4fv(uModelMat, 1, true, ModelMat);
	glUniformMatrix4fv(uProjMat, 1, true, ProjMat);

	glUniform4f(uLPos, lPos[0], lPos[1], lPos[2], lPos[3]);
	glUniform4f(uAmb, amb[0], amb[1], amb[2], amb[3]);
	glUniform4f(uDif, dif[0], dif[1], dif[2], dif[3]);
	
	glUniform1i(uTex, 1);
	glUniform1i(uMapTex, 0);
	glUniform4f(uEPos, ePos[0], ePos[1], ePos[2], 1);

	if(drawingMode == 1)
		sphere.Draw(phong_prog);
	else if (drawingMode == 2)
		bunny.Draw(phong_prog);
	
	glutSwapBuffers();
}

bool play = true;

void idle()
{
	if(play)
	{
		g_Time += 0.016;
		Sleep(16);
		glutPostRedisplay();
	}
}

void keyboard(unsigned char ch, int x, int y)
{
	if (ch == '3') {
		isDiffuseMap = !isDiffuseMap;
		printf("DiffuseMap %s\n", isDiffuseMap ? "on" : "off");
	}
		
	else if (ch == 'q') {
		drawingMode += 1;
		drawingMode %= 3;
	}
	else if (ch == '2') {
		fresnel += 0.5;
		fresnel = std::min(fresnel, 11.0f);
		printf("fresnel = %f\n", fresnel);
	}
	else if (ch == '1') {
		fresnel -= 0.5;
		fresnel = std::max(fresnel, 0.5f);
		printf("fresnel = %f\n", fresnel);
	}

}
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		isDraggingLeft = true;
		lastMousePos = vec2(x, y);
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		isDraggingLeft = false;
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		isDraggingRight = true;
		lastMousePos = vec2(x, y);
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		isDraggingRight = false;
	}
}

void motion(int x, int y)
{
	vec2 currentMousePos = vec2(x, y);
	vec2 delta = currentMousePos - lastMousePos;

	if (isDraggingLeft)
	{
		alpha += delta.x * 0.1f;
		beta -= delta.y * 0.1f;

		beta = std::min(std::max(beta, -89.0f), 89.0f);
		float radYaw = alpha * 3.141592f / 180.0f;
		float radPitch = beta * 3.141592f / 180.0f;

		ePos.x = cameraDistance * cos(radPitch) * sin(radYaw);
		ePos.y = cameraDistance * sin(radPitch);
		ePos.z = cameraDistance * cos(radPitch) * cos(radYaw);
	}
	else if (isDraggingRight)
	{
		cameraDistance -= delta.y * 0.03f;
		cameraDistance = std::max(cameraDistance, 2.0f);
		cameraDistance = std::min(cameraDistance, 8.0f); 

		float radAlpha = alpha * 3.141592f / 180.0f;
		float radBeta = beta * 3.141592f / 180.0f;

		ePos.x = cameraDistance * cos(radBeta) * sin(radAlpha);
		ePos.y = cameraDistance * sin(radBeta);
		ePos.z = cameraDistance * cos(radBeta) * cos(radAlpha);
	}
	
	GLuint uModelMat = glGetUniformLocation(phong_prog, "uModelMat");
	GLuint uEPos = glGetUniformLocation(phong_prog, "uEPos");

	ModelMat = myLookAt(ePos, vec3(0, 0, 0), vec3(0, 1, 0));
	glUniform4f(uEPos, ePos[0], ePos[1], ePos[2], 1);
	glUniformMatrix4fv(uModelMat, 1, true, ModelMat);
	
	lastMousePos = currentMousePos;
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	
	g_aspect = w/float(h);
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	std::cout << "Input Image Group Name (ex: class1 / class2/ ny / lions / elephant / glacier / ice / waterfall): ";
	std::cin >> groupName;
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(800,500);

	glutCreateWindow("HW5");

	glewExperimental = true;
	glewInit();

	myInit();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMainLoop();

	return 0;
}
