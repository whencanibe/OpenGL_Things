#include <vgl.h>
#include <InitShader.h>
#include "MyCube.h"
#include "MyObj.h"

#include <vec.h>
#include <mat.h>
#include <algorithm>

MyCube cube;
MyObj obj;

GLuint program;
GLuint prog_phong;

GLuint uMat(1.0f);
GLuint uColor;
mat4 g_Mat = mat4(1.0f);

int winWidth = 500;
int winHeight = 500;

bool isRotating = false;
int rotationAxis = 0; // 0: X, 1: Y, 2: Z

mat4 R = mat4(1.0f);
bool bprint = true;
mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	vec3 n = normalize(at - eye); //front and back
	vec3 u = dot(up, n) * n;
	vec3 v = normalize(up - u); // up and down
	vec3 w = normalize(cross(n, v)); //left and right

	mat4 Rw(1.0f);
	Rw[0][0] = w.x; Rw[0][1] = v.x; Rw[0][2] = -n.x;
	Rw[1][0] = w.y; Rw[1][1] = v.y; Rw[1][2] = -n.y;
	Rw[2][0] = w.z; Rw[2][1] = v.z; Rw[2][2] = -n.z;

	mat4 Rc(1.0f); //Transpose
	Rc[0][0] = w.x; Rc[0][1] = w.y; Rc[0][2] = w.z;
	Rc[1][0] = v.x; Rc[1][1] = v.y; Rc[1][2] = v.z;
	Rc[2][0] = -n.x; Rc[2][1] = -n.y; Rc[2][2] = -n.z;

	mat4 eyeT = Translate(-eye.x,-eye.y,-eye.z);
	mat4 Rcc(1.0f);
	Rcc = Rc * eyeT;

	if(bprint)
	for (int i = 0; i < 4; i++) {
		printf("%.2f %.2f %.2f %.2f\n", Rcc[i][0], Rcc[i][1], Rcc[i][2], Rcc[i][3]);
		
	}
	bprint = false;
	return Rc * eyeT;
}

mat4 myPerspective(float fovy, float aspectRatio, float zNear, float zFar)
{
	float theta = fovy * 3.141592 / 180.0f;
	float ymax = zFar * tan(theta / 2);
	float ymin = -ymax;
	float xmax = ymax * aspectRatio;
	float xmin = -xmax;

	mat4 S = Scale(2.0f/(xmax-xmin),2.0f/(ymax-ymin),1.0f/zFar);

	float c = -1 * zNear / zFar;
	mat4 H(1.0f);
	H[2][2] = 1.0f / (c + 1); H[2][3] = -c / (c + 1);
	H[3][2] = -1.0f;		  H[3][3] = 0;

	return H * S;
}


void myInit()
{
	cube.Init();
	obj.Init();

	obj.updateGpu();
	program = InitShader("vshader.glsl", "fshader.glsl");
	prog_phong = InitShader("vphong.glsl", "fphong.glsl");
}

void DrawAxis()
{
	glUseProgram(program);
	mat4 x_a= Translate(0.5,0,0)*Scale(1,0.03,0.03);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*x_a);
	glUniform4f(uColor, 1, 0, 0, 1);
	cube.Draw(program);

	mat4 y_a= Translate(0,0.5,0)*Scale(0.03,1,0.03);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*y_a);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);

	mat4 z_a= Translate(0,0,0.5)*Scale(0.03,0.03,1);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat*z_a);
	glUniform4f(uColor, 0, 0, 1, 1);
	cube.Draw(program);
}

float g_Time = 0;
float KS = 0.5;
float SH = 30;

void display()
{
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	vec3 eye3(2, 0, 0);
	vec3 at(0, 0, 0);
	vec3 up(0, 1, 0);

	mat4 V = myLookAt(eye3, at, up);
	mat4 M = R;
	mat4 P = myPerspective(60.0f, 1.0f, 0.1, 100);
	
	g_Mat = P * V * M;

	glUseProgram(program);
	uMat = glGetUniformLocation(program, "uMat");
	uColor = glGetUniformLocation(program, "uColor");

	DrawAxis();
	
	glUseProgram(prog_phong);

	vec3 center = (obj.max + obj.min)/2;
	float maxFactor = std::max({obj.max.x - obj.min.x,obj.max.y - obj.min.y ,obj.max.z - obj.min.z });
	mat4 S = Scale(1.5 / maxFactor);
	M = R * S *Translate(-center);

	GLuint uProjMat = glGetUniformLocation(prog_phong, "uProjMat");
	GLuint uModelMat = glGetUniformLocation(prog_phong, "uModelMat");
	GLuint ka = glGetUniformLocation(prog_phong, "ka");
	GLuint kd = glGetUniformLocation(prog_phong, "kd");
	GLuint ks = glGetUniformLocation(prog_phong, "ks");

	GLuint shine = glGetUniformLocation(prog_phong, "shine");
	GLuint I = glGetUniformLocation(prog_phong, "I");
	GLuint lightPos = glGetUniformLocation(prog_phong, "lightPos");
	
	GLuint cam = glGetUniformLocation(prog_phong, "cam");


	glUniformMatrix4fv(uProjMat, 1, true, P);
	glUniformMatrix4fv(uModelMat, 1, true,V * M);
	glUniform4f(ka, 0.1, 0.1, 0.2, 1);
	glUniform4f(kd, 0.6, 0.6, 0.6, 1);
	glUniform4f(ks, KS, KS, KS, 1);
	glUniform4f(I, 1.0, 1.0, 1.0, 1);
	glUniform4f(lightPos, 5.0, 5.0, 5.0, 1);
	glUniform1f(shine, SH);
	glUniform4f(cam, 0, 0, 0, 1);

	obj.Draw(prog_phong);
	
	glutSwapBuffers();
}


void idle()
{
	g_Time += 0.016;
	if (isRotating) {
		mat4 rotationDirection;
		if (rotationAxis == 0) rotationDirection = RotateX(1.0f);
		else if (rotationAxis == 1) rotationDirection = RotateY(1.0f);
		else if (rotationAxis == 2) rotationDirection = RotateZ(1.0f);

		R = rotationDirection * R; 
	}

	Sleep(16);					// for vSync
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0,0,w,h);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	if (key == '1') {
		obj.normalMode(false);
		obj.updateGpu(); 
	}
	else if (key == '2') {
		obj.normalMode(true);
		obj.updateGpu(); 
	} 
	else if (key == '3') {
		if (KS < 1.5f)
			KS += 0.1f;
		printf("Increase ks\n");
	}
	else if (key == '4') {
		if (KS >= 0.1)
			KS -= 0.1f;
		printf("decrease ks\n");
	}
	else if (key == '5') {
		if (SH < 120.0f)
			SH += 10.0f;
		printf("increase shininess\n");
	}
	else if (key == '6') {
		if (SH >= 20.0f)
			SH -= 10.0f;
		printf("decrease shininess\n");
	}
	else if (key == ' ') {
		isRotating = !isRotating;
		std::cout << (isRotating ? "play" : "stop") << std::endl;
	}

	else if (key == 'q' || key == 'Q') {
		exit(0);
	}
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON) {
		std::cout << "Left:x" << std::endl;
		rotationAxis = 0;
	}
	else if (button == GLUT_MIDDLE_BUTTON) {
		std::cout << "Middle:y" << std::endl;
		rotationAxis = 1;
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		std::cout << "Right:z" << std::endl;
		rotationAxis = 2;
	}
}

void getFilePath() {
	std::string objFilePath;

	while (true) {
		std::cout << "Input File Name : ";
		std::getline(std::cin, objFilePath);

		std::ifstream file(objFilePath);
		if (!file.is_open()) {
			std::cout << "File not found. Please try again." << std::endl;
			continue; 
		}

		file.close();

		if (!obj.loadFile(objFilePath)) {
			std::cout << "Failed to load the file. Please try again." << std::endl;
			continue; 
		}

		std::cout << "Successed loading : " << objFilePath << std::endl;
		break;
	}
}


int main(int argc, char ** argv)
{
	glutInit(&argc, argv);

	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
		glGetString(GL_SHADING_LANGUAGE_VERSION));

	printf("SIMPLE OBJ MODEL VIEWER\n");
	printf("----------------------------------------------------------------\n");
	printf("Spacebar: starting/stoping rotation\n\n");
	printf("Left Mouse Button: rotating around x-axis\n");
	printf("Middle Mouse Button: rotating around y-axis\n");
	printf("Right Mouse Button: rotating around z-axis\n\n");
	printf("'1' key: Using Vertex Normal for shading\n");
	printf("'2' key: Using Surface Normal for shading\n");
	printf("'3' key: Increasing Specular effect (ks)\n");
	printf("'4' key: Decreasing Specular effect (ks)\n");
	printf("'5' key: Increasing Shininess (n)\n");
	printf("'6' key: Decreasing Shininess (n)\n\n");
	printf("'Q' Key: Exit the program.\n");
	printf("----------------------------------------------------------------\n");

	//getFilePath();
	obj.loadFile("bunny.obj");
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(winWidth,winHeight);

	glutCreateWindow("Assignment4");

	glewExperimental = true;
	glewInit();

	


	myInit();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}
