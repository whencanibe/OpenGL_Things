
#define _CRT_SECURE_NO_WARNINGS

#include <vgl.h>
#include <InitShader.h>
#include "MyCube.h"
#include "MyUtil.h"

#include <vec.h>
#include <mat.h>
#include <queue>

#define MAZE_FILE	"maze.txt"

MyCube cube;
GLuint program;

mat4 g_Mat = mat4(1.0f);
GLuint uMat;
GLuint uColor;

float wWidth = 1000;
float wHeight = 500;

vec3 cameraPos = vec3(0, 0, 0);
vec3 viewDirection = vec3(0, 0, -1);
vec3 goalPos = vec3(0, 0, 0);

int MazeSize;
char maze[255][255] = { 0 };

float cameraSpeed = 0.1f;
float cameraRotateSpeed = 3.0f;
float g_time = 0;

std::vector<vec3> findPath;
bool showPath = false;
bool isFollowing = false;
bool isWall = false;

inline vec3 getPositionFromIndex(int i, int j)
{
	float unit = 1;
	vec3 leftTopPosition = vec3(-MazeSize / 2.0 + unit / 2, 0, -MazeSize / 2.0 + unit / 2);
	vec3 xDir = vec3(1, 0, 0);
	vec3 zDir = vec3(0, 0, 1);
	return leftTopPosition + i * xDir + j * zDir;
}

void LoadMaze()
{
	FILE* file = fopen(MAZE_FILE, "r");
	char buf[255];
	fgets(buf, 255, file);
	sscanf(buf, "%d", &MazeSize);
	for (int j = 0; j < MazeSize; j++)
	{
		fgets(buf, 255, file);
		for (int i = 0; i < MazeSize; i++)
		{
			maze[i][j] = buf[i];
			if (maze[i][j] == 'C')				// Setup Camera Position
				cameraPos = getPositionFromIndex(i, j);
			if (maze[i][j] == 'G')				// Setup Goal Position
				goalPos = getPositionFromIndex(i, j);
		}
	}
	fclose(file);
}

bool isCollide(vec3 circleCenter, float circleRadius, vec3 rectCenter, float rectWidth, float rectHeight) {
	float rectLeft = rectCenter.x - rectWidth / 2.0f;
	float rectRight = rectCenter.x + rectWidth / 2.0f;
	float rectBottom = rectCenter.z - rectHeight / 2.0f;
	float rectTop = rectCenter.z + rectHeight / 2.0f;

	float closestX = std::max(rectLeft, std::min(circleCenter.x, rectRight));
	float closestZ = std::max(rectBottom, std::min(circleCenter.z, rectTop));

	float distance = length(vec3(closestX, 0, closestZ) - circleCenter);

	return distance <= circleRadius;
}


void DrawMaze()
{
	for (int j = 0; j < MazeSize; j++)
		for (int i = 0; i < MazeSize; i++)
			if (maze[i][j] == '*')
			{
				vec3 wallPos = getPositionFromIndex(i, j);
				vec3 color;

				//collision
				float cameraRadius = 0.25;
				float wallWidth = 1.0f;
				if (isCollide(cameraPos, cameraRadius, wallPos, wallWidth, wallWidth)) {
					isWall = true;
					cameraPos += (cameraPos - wallPos) * cameraSpeed; 
					color = vec3(1, 0, 0); 
				}
				else {
					isWall = false;
					color = vec3(i / (float)MazeSize, j / (float)MazeSize, 1);
				}
				mat4 ModelMat = Translate(getPositionFromIndex(i, j));
				glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
				glUniform4f(uColor, color.x, color.y, color.z, 1);
				cube.Draw(program);
			}
}

void myInit()
{
	LoadMaze();
	cube.Init();
	program = InitShader("vshader.glsl", "fshader.glsl");

}

struct Node {
	int x, z;
	float gCost;
	float hCost;

	float fCost() const {
		return gCost + hCost;
	}

	bool operator>(const Node& other) const {
		return fCost() > other.fCost();
	}

	Node(int x, int z, int gCost, int hCost) : x(x), z(z), gCost(gCost), hCost(hCost) {}
};

float Manhattan(int x1, int z1, int x2, int z2) {
	return abs(x1 - x2) + abs(z1 - z2);
}
/*
float Euclidean(int x1, int z1, int x2, int z2) {
	return sqrt((x1 - x2) * (x1 - x2) + (z1 - z2)* (z1 - z2));
}
*/

void AStar() {
	std::pair<int, int> prevIndex[255][255];
	std::priority_queue<Node, std::vector<Node>, std::greater<Node>> minCost;

	
	Node start = Node(int(cameraPos.x + MazeSize / 2.0f), int(cameraPos.z + MazeSize / 2.0f), 0, 0);
	Node goal = Node(int(goalPos.x + MazeSize / 2.0f), int(goalPos.z + MazeSize / 2.0f), 0, 0);

	minCost.push(start);
	prevIndex[start.x][start.z] = { -1,-1 };

	bool visited[255][255] = { false };
	findPath.clear();

	while (!minCost.empty()) {
		Node current = minCost.top();
		minCost.pop();

		if (visited[current.x][current.z]) continue;

		visited[current.x][current.z] = true;

		if (current.x == goal.x && current.z == goal.z) {
			int px = goal.x;
			int pz = goal.z;
			while (px != -1) {
				findPath.push_back(getPositionFromIndex(px, pz));
				int tempX = prevIndex[px][pz].first;
				int tempZ = prevIndex[px][pz].second;
				px = tempX;
				pz = tempZ;
			}
			std::reverse(findPath.begin(), findPath.end());
			return;
		}

		int dir[4][2] = { {0,1}, {-1,0}, {0,-1}, {1,0} };
		for (int i = 0; i < 4; i++) {
			int nextX = current.x + dir[i][0];
			int nextZ = current.z + dir[i][1];
			
			if (!visited[nextX][nextZ] && maze[nextX][nextZ] != '*')
			{
				int nextG = current.gCost + 1;
				int nextH = Manhattan(nextX, nextZ, goal.x, goal.z);

				Node next = Node(nextX, nextZ, nextG, nextH);
				minCost.push(next);
				prevIndex[nextX][nextZ] = std::make_pair(current.x, current.z);
			}
		}
	}
}

void DrawGrid()
{
	float n = 40;
	float w = MazeSize;
	float h = MazeSize;

	for (int i = 0; i < n; i++)
	{
		mat4 m = Translate(0, -0.5, -h / 2 + h / n * i) * Scale(w, 0.02, 0.02);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 1, 1, 1, 1);
		cube.Draw(program);
	}
	for (int i = 0; i < n; i++)
	{
		mat4 m = Translate(-w / 2 + w / n * i, -0.5, 0) * Scale(0.02, 0.02, h);
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 1, 1, 1, 1);
		cube.Draw(program);
	}
}


void drawCamera()
{
	float cameraSize = 0.5;
	float ang = atan2(viewDirection.x, viewDirection.z) * 180 / 3.141592;

	mat4 ModelMat = Translate(cameraPos) * RotateY(ang) * Scale(vec3(cameraSize));
	glUseProgram(program);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);

	ModelMat = Translate(cameraPos + viewDirection * cameraSize / 2) * RotateY(ang) * Scale(vec3(cameraSize / 2));
	glUseProgram(program);
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 1, 0, 1);
	cube.Draw(program);
}

void drawGoal()
{
	glUseProgram(program);
	float GoalSize = 0.7;

	mat4 ModelMat = Translate(goalPos) * RotateY(g_time * 3) * Scale(vec3(GoalSize));
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 0, 0, 0);
	cube.Draw(program);

	ModelMat = Translate(goalPos) * RotateY(g_time * 3 + 45) * Scale(vec3(GoalSize));
	glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * ModelMat);
	glUniform4f(uColor, 0, 0, 0, 0);
	cube.Draw(program);
}

void drawPath() {
	for (int i = 0; i < findPath.size()-1; i++)
	{
		vec3 startP = findPath[i];
		vec3 endP = findPath[i + 1];
		mat4 m;
		if (startP.x == endP.x) {
			m = Translate(startP.x,-0.5,startP.z + (endP.z - startP.z) / 2) * Scale(0.06, 0.06, length(endP - startP));
		}
		else if (startP.z == endP.z) {
			m = Translate(startP.x + (endP.x - startP.x) / 2, -0.5, startP.z) * Scale(length(endP - startP), 0.06, 0.06);
		}
		
		glUniformMatrix4fv(uMat, 1, GL_TRUE, g_Mat * m);
		glUniform4f(uColor, 0.5 + 0.5 * cos(g_time), 0, 0, 1);
		
		cube.Draw(program);
	}
}


void drawScene(bool bDrawCamera = true)
{
	glUseProgram(program);
	uMat = glGetUniformLocation(program, "uMat");
	uColor = glGetUniformLocation(program, "uColor");

	DrawGrid();
	DrawMaze();
	drawGoal();

	if (bDrawCamera)
		drawCamera();

	if (showPath) drawPath();
}

void display()
{
	glEnable(GL_DEPTH_TEST);

	float vWidth = wWidth / 2;
	float vHeight = wHeight;

	// LEFT SCREEN : View From Camera (Perspective Projection)
	glViewport(0, 0, vWidth, vHeight);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	float h = 4;
	float aspectRatio = vWidth / vHeight;
	float w = aspectRatio * h;
	mat4 ViewMat = myLookAt(cameraPos, cameraPos + viewDirection, vec3(0, 1, 0));
	mat4 ProjMat = myPerspective(45, aspectRatio, 0.01, 20);

	g_Mat = ProjMat * ViewMat;
	drawScene(false);							// drawing scene except the camera


	// RIGHT SCREEN : View from above (Orthographic parallel projection)
	glViewport(vWidth, 0, vWidth, vHeight);
	h = MazeSize;
	w = aspectRatio * h;
	ViewMat = myLookAt(vec3(0, 5, 0), vec3(0, 0, 0), vec3(0, 0, -1));
	ProjMat = myOrtho(-w / 2, w / 2, -h / 2, h / 2, 0, 20);

	g_Mat = ProjMat * ViewMat;
	drawScene(true);


	glutSwapBuffers();
}


// input <-- 1 : left or -1: right
void rotateCam(int direction) {
	vec4 rotate = RotateY(direction * cameraRotateSpeed) * vec4(viewDirection, 0.0f);
	viewDirection.x = rotate.x;
	viewDirection.z = rotate.z;
	viewDirection.y = 0;
	viewDirection = normalize(viewDirection);
}

int nextPosIndex = 0;
void followPath() {
	if (nextPosIndex >= findPath.size()) {
		isFollowing = false;
		return;
	}
	if (length(cameraPos - goalPos) >= cameraSpeed)
	{
		if (length(findPath[nextPosIndex] - cameraPos) < 0.01f && nextPosIndex < findPath.size() - 1)
			nextPosIndex++;

		if (abs(dot(viewDirection, normalize(findPath[nextPosIndex] - cameraPos)) - 1) < 0.01f)
		{
			viewDirection = normalize(findPath[nextPosIndex] - cameraPos);
			cameraPos += cameraSpeed * viewDirection;
		}
		else
		{
			if (cross(findPath[nextPosIndex] - cameraPos, viewDirection).y < 0)
				rotateCam(1);
			else if (cross(findPath[nextPosIndex] - cameraPos, viewDirection).y > 0)
				rotateCam(-1);
		}
	}
	else {
		isFollowing = !isFollowing;
	}
}

void myKeyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'q' : 
			if(!showPath && length(cameraPos - goalPos) > 0.01f)
				AStar();

			if(!isFollowing)
				showPath = !showPath;
			break;
		case ' ':
			
			if (!showPath) break; //if the path is not showed, don't let it move
			else if (showPath && !isFollowing) { //make the cam start following the path
				isFollowing = !isFollowing;
				cameraPos = findPath[0];
				nextPosIndex = 1;
				vec3 nextPos = findPath[nextPosIndex];
				viewDirection = normalize(nextPos - cameraPos);
			}
			else { // make the cam stop
				isFollowing = !isFollowing;
			}
			break;
	}
}

void idle()
{
	g_time += 1;
	if (isFollowing) followPath();
	else {
		if ((GetAsyncKeyState('A') & 0x8000) == 0x8000)		// if "A" key is pressed	: Turn Left
			rotateCam(1);
		if ((GetAsyncKeyState('D') & 0x8000) == 0x8000)		// if "D" key is pressed	: Turn Right
			rotateCam(-1);
		if ((GetAsyncKeyState('W') & 0x8000) == 0x8000)		// if "W" key is pressed	: Go Forward
				cameraPos += cameraSpeed * viewDirection;
		if ((GetAsyncKeyState('S') & 0x8000) == 0x8000)		// if "S" key is pressed	: Go Backward
				cameraPos += cameraSpeed * viewDirection * -1;
	}
	
	Sleep(16);											// for vSync
	glutPostRedisplay();
}

void reshape(int wx, int wy)
{
	printf("%d %d \n", wx, wy);
	wWidth = wx;
	wHeight = wy;
	glutPostRedisplay();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(wWidth, wHeight);

	glutCreateWindow("Homework3 (Maze Navigator)");

	glewExperimental = true;
	glewInit();

	printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
		glGetString(GL_SHADING_LANGUAGE_VERSION));

	myInit();
	glutDisplayFunc(display);
	glutKeyboardFunc(myKeyboard);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutMainLoop();

	return 0;
}