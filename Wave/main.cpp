#include <vgl.h>
#include <InitShader.h>
#include <vec.h>
#include "MyPlain.h"
#include <stdio.h>

bool waving = false;
bool rotating = false;

GLuint program;
MyPlain plain;
int initDivision = 30;
void myInit()
{
	plain.init(initDivision);
	program = InitShader("vShader.glsl", "fShader.glsl");
}

float myTime = 0;

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(program);

	GLuint uTime = glGetUniformLocation(program, "uTime");
	glUniform1f(uTime, myTime);

	GLuint uWaving = glGetUniformLocation(program, "uWaving");
	glUniform1f(uWaving, waving);

	plain.draw(program);
	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		waving = !waving;
		break;
	case ' ':
		rotating = !rotating;
		
		break;
	case '1':
		plain.decrease();
		break;
	case '2':
		plain.increase();
		break;
	case 'q':
		exit(0);
		break;
	}
}

void idle()
{
	if(rotating)myTime += 0.0333f;
	Sleep(33);		
	glutPostRedisplay();
}

int main(int argc, char** argv)
{	
	
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("A Waving Color Plane");

	printf("A Waving Color Plane\n");
	printf("---------------------------------------------------\n");
	printf("`1` key: Decreasing the number of Division\n");
	printf("`2` key: Increasing the number of Division\n");
	printf("`w` key: Showing/hiding the waving pattern\n");
	printf("Spacebar: Starting/stoping rotating and waving\n");
	printf("\n");
	printf("`q` key: Exit the program\n");
	printf("---------------------------------------------------\n");
	printf("\n");

	glewExperimental = true;
	glewInit();
	myInit();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);

	glutMainLoop();

	return 0;
}