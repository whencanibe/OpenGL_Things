#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include "MyCube.h"
#include "MyPyramid.h"
#include "MyTarget.h"

MyCube cube;
MyPyramid pyramid;
MyTarget target(&cube);

GLuint program;
GLuint uMat;

mat4 CTM;

bool bPlay = false;
bool bChasingTarget = false;
bool bDrawTarget = false;

float ang1 = 0;
float ang2 = 0;
float ang3 = 0;
float radian = 3.141592 / 180.0f;


void myInit()
{
	cube.Init();
	pyramid.Init();

	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
}

float g_time = 0;

void drawRobotArm(float ang1, float ang2, float ang3)
{
	mat4 temp = CTM;

	// BASE
	mat4 M(1.0);

	M = Translate(0, 0, 0.075) * Scale(0.3, 0.3, 0.05);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	pyramid.Draw(program);

	M = Translate(0, 0, -0.075) * Scale(0.3, 0.3, 0.05);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	pyramid.Draw(program);

	//joint1
	M = Scale(0.05, 0.05, 0.25);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	// Upper Arm
	CTM *= RotateZ(ang1); //ang1

	M = Translate(0, 0.2, 0) * Scale(0.1, 0.5, 0.1); // translate y : 0.25 - 0.05
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	float jointWidth2 = 0.05; // To change the width of joint

	// Lower Arm
	CTM *= Translate(0, 0.4, 0) * RotateZ(ang2); //ang2 // translate y : 0.5 - 0.1

	M = Translate(0, 0.2 - jointWidth2, 0.075) * Scale(0.1, 0.4, 0.05); //translate y : 0.2 - 0.05
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	M = Translate(0, 0.2 - jointWidth2, -0.075) * Scale(0.1, 0.4, 0.05); //translate y : 0.2 - 0.05
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	//joint2
	M = Scale(jointWidth2, jointWidth2, 0.25);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	// Hand
	CTM *= Translate(0, 0.3, 0) * RotateZ(ang3);//ang3 //0.4 - 0.1

	M = Scale(0.35, 0.15, 0.1);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	//joint3
	M = Scale(jointWidth2, jointWidth2, 0.25);
	glUniformMatrix4fv(uMat, 1, true, CTM * M);
	cube.Draw(program);

	CTM = temp;
}
float AngleBetween(vec3 a, vec3 b) {
	float cos = dot(a, b) / (length(a) * length(b));
	float angle = 0;
	if (cos >= -1 && cos <= 1) angle = acos(cos) / radian;

	if (cross(a, b).z < 0) angle = -angle;

	return angle;
}

void computeAngle()
{
	vec3 targetPos = target.GetPosition(g_time);
	float x = targetPos.x;
	float y = targetPos.y;

	float errorRange = 0.01f;
	int maxIteration = 100; // max number for iteration
	int iteration = 0;

	vec3 joint2;
	vec3 joint3;
	vec3 endEffector;

	while (iteration < maxIteration)
	{
		vec4 t_joint2 = RotateZ(ang1) * Translate(0, 0.4, 0) * vec4(0, 0, 0, 1);
		vec4 t_joint3 = RotateZ(ang1) * Translate(0, 0.4, 0)
			* RotateZ(ang2) * Translate(0, 0.3, 0) * vec4(0, 0, 0, 1);
		vec4 t_endEffector = RotateZ(ang1) * Translate(0, 0.4, 0)
			* RotateZ(ang2) * Translate(0, 0.3, 0) * RotateZ(ang3) * Translate(0.175, 0, 0) * vec4(0, 0, 0, 1);

		joint2.x = t_joint2.x; joint2.y = t_joint2.y; joint2.z = 0;
		joint3.x = t_joint3.x; joint3.y = t_joint3.y; joint3.z = 0;
		endEffector.x = t_endEffector.x; endEffector.y = t_endEffector.y; endEffector.z = 0;

		vec3 joint3ToEnd = endEffector - joint3;
		vec3 joint3ToTarget = targetPos - joint3;

		ang3 += AngleBetween(joint3ToEnd, joint3ToTarget) / 100;

		vec3 joint2ToEnd = endEffector - joint2;
		vec3 joint2ToTarget = targetPos - joint2;

		ang2 += AngleBetween(joint2ToEnd, joint2ToTarget) / 100;

		vec3 joint1ToEnd = endEffector;
		vec3 joint1ToTarget = targetPos;

		ang1 += AngleBetween(joint1ToEnd, joint1ToTarget) / 100;

		float targetDistance = sqrt((endEffector.x - x) * (endEffector.x - x) +
			(endEffector.y - y) * (endEffector.y - y));


		if (targetDistance <= errorRange) break;

		iteration++;
	}
}



void myDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	GLuint uColor = glGetUniformLocation(program, "uColor");
	glUniform4f(uColor, -1, -1, -1, -1);


	uMat = glGetUniformLocation(program, "uMat");
	CTM = Translate(0, -0.4, 0) * RotateY(g_time * 30);
	drawRobotArm(ang1, ang2, ang3);


	glUniform4f(uColor, 1, 0, 0, 1);
	if (bDrawTarget == true)
		target.Draw(program, CTM, g_time);

	glutSwapBuffers();
}

void myIdle()
{
	if (bPlay)
	{
		g_time += 1 / 60.0f;
		Sleep(1 / 60.0f * 1000);

		if (bChasingTarget == false)
		{
			ang1 = 45 * sin(g_time * 3.141592);
			ang2 = 60 * sin(g_time * 2 * 3.141592);
			ang3 = 30 * sin(g_time * 3.141592);
		}
		else
			computeAngle();

		glutPostRedisplay();
	}
}

void myKeyboard(unsigned char c, int x, int y)
{

	switch (c)
	{
	case '1':
		bChasingTarget = !bChasingTarget;
		break;
	case '2':
		bDrawTarget = !bDrawTarget;
		break;
	case '3':
		target.toggleRandom();
		break;
	case ' ':
		bPlay = !bPlay;
		break;
	default:
		break;
	}
}


int main(int argc, char** argv)
{
	printf("SIMPLE INTELLIGENT ROBOT ARM\n");
	printf("\n");
	printf("-----------------------------------------------------------\n");
	printf("`1' key: Toggle Between Chasing the target or not\n");
	printf("`2' key: Toggle Between Showing the target position or not\n");
	printf("`3' key: Toggle Between Randomly Positioning the target or not\n");
	printf("\n");
	printf("Spacebar : start/stop Playing\n");
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Simple Robot Arm");

	glewExperimental = true;
	glewInit();

	myInit();
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKeyboard);
	glutIdleFunc(myIdle);

	glutMainLoop();

	return 0;
}