#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "global.h"
#include "Object.h"
#include "Line.h"
#include "CubeMap.h"
#include "Snow.h"
#include "Arrow.h"
#include "Bow.h"
#include "Grass.h"
#include "Circle.h"
#include "Board.h"
#include "Cube.h"

#include "Common.h"

char* SERVERIP = (char*)"127.0.0.2"; // default local
#define SERVERPORT 9000
const int CIRCLENUMWIDTH = 2;
const int CIRCLENUMHEIGHT = 2;
const int CIRCLENUM = CIRCLENUMWIDTH * CIRCLENUMHEIGHT;

const float acc = 0.01;
const float arrowMoveZ = 0.5;

const int snowSize = 10001;
const int snowSizeDiv = 100;
const float snowStartX= 50.0;
const float snowStartY= 65.0;
const float particleAcc = 0.0005;
const float particleSubZ = 0.005;


char* filetobuf(const char* file);
void InitBuffer();
void InitShader();
void Timer(int value);
int make_vertexShaders();
int make_fragmentShaders();
void Convert_xy(int x, int y);
void InitTexture();
void key_check();

GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Motion(int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid mouseWheel(int button, int dir, int x, int y);
GLvoid Reshape(int w, int h);
GLvoid Keyborad(unsigned char key, int x, int y);
GLvoid Keyborad_up(unsigned char key, int x, int y);

bool CreateSocket();

SOCKET sock;
sockaddr_in serveraddr;
int retval;

glm::mat4 cameratransform;
glm::mat4 cameratransform1;
glm::mat4 cameratransform2;
glm::mat4 cameratransform3;
glm::mat4 cameratransform4;
glm::mat4 cameratransform5;
glm::mat4 cameratransform6;

glm::mat4 cameratransform7;
glm::mat4 cameratransform9;

glm::mat4 cameratransform8;
glm::mat4 cameratransform10;

glm::mat4 cameratransform11;

unsigned int texture[45];

GLuint shaderID;
GLint width, height;
GLuint s_program;
GLchar* vertexsource;
GLuint vertexShader;
GLchar* fragmentsource;
GLuint fragmentShader;


Snow snow[SNOW_SIZE];
Grass grass[GRASS_SIZE];
CubeMap background;
Arrow arrow;
Circle circle[10];

Circle **circles;
int circlewidth;
int circleheight;

Line line;
Board board;
Bow bow;
Cube paticle[CUBE_SIZE];

Arrow otherArrow;
Bow otherBow;

int dwID;

float other_x_angle = 0;
float other_y_angle = 0;

float ox = 0, oy = 0;
float x_angle = 0;
float y_angle = 0;
float z_angle = 0;
float pre_x_angle = 0;
float pre_y_angle = 0;
float wheel_scale = 0.0;
bool left_button = 0;
float fovy = 45;
float near_1 = 0.1;
float far_1 = 200.0;
float persfect_z = -2.0;
float camera_x = 0.0;
float camera_y = 0.0;
float camera_z = 0.0;

float otherCamera_x = 0.0;
float otherCamera_y = 0.0;
float otherCamera_z = 0.0;
glm::vec3 cameraPos{ camera_x, camera_y, camera_z };
glm::vec3 otherCameraPos{ camera_x, camera_y, camera_z };

bool keybuffer[256] = { 0, };
float Light_R = 1.0f;
float Light_G = 1.0f;
float Light_B = 1.0f;

float x_1 = 0.0;
float y_1 = -1.0;
float z_1 = -0.0;

float t = 0;
float arrow_angle_x = 0;
float arrow_angle_y = 0;
float v = 0;
float arrow_x = 0;
float arrow_z = 0;
float arrow_y = 0;
float pre_arrow_x = 0;
float pre_arrow_z = 0;
float pre_arrow_y = 0;
bool arrow_on = false;
float distance = 0;
float wind_x = 0;
float wind_y = 0;
float wind_z = 0;
float wind_speed = 0.0;
int wind_dir = 0;
int wind_timer = 1000;

int camera_mode = 0;
int score = 0;
short total_score = 0;
int stage = 0;
bool score_on = false;
bool replay = false;
bool pass = false;

float particle_during = 1.0;
float particleSpeed = 0;
bool particle_on = false;
int particle_way_x[CUBE_SIZE];
int particle_way_y[CUBE_SIZE];
glm::vec3 hitPos = { 0,0,0 };

int myScore = 0;
int otherScore = 0;

int myWin = 0;
int otherWin = 0;

int number_1 = 0;
int number_10 = 0;

float wind_angle_z = 0;

bool main_loading = true;

GLenum Mode = GL_FILL;

bool connectState = false;
HANDLE DataThread;

struct Packet {
	float x_angle, y_angle; // 플레이어의 시야 각도 값
	glm::vec3 arrowPosition; // 화살의 좌표 값
	glm::vec3 arrowRotation;
};

struct InPacket {
	float x_angle, y_angle; // 플레이어의 시야 각도 값
	glm::vec3 arrowPosition; // 화살의 좌표 값;
	glm::vec3 arrowRotation; // 화살의 회전 값;
	short total_score; // 플레이어의 현재 점수
	short wind_dir; // 바람의 방향
	float wind_speed; // 바람의 세기
	short circleState[CIRCLENUM]; // 과녁의 상태
	int stage; // 스테이지
	short winCount; // 승리 카운트
};

struct InitPacket {
	glm::vec3 circleCenter[CIRCLENUM]; // 과녁의 중앙의 위치 값
	glm::vec3 player1Pos; // 플레이어 1의 위치
	glm::vec3 player2Pos; // 플레이어 2의 위치
};

InitPacket initPacket;
InPacket inPacket;

DWORD WINAPI DataComm(LPVOID arg)
{
	while (1) {
		if (connectState) {
			Packet packet;
			packet.x_angle = x_angle;
			packet.y_angle = y_angle;
			packet.arrowPosition = arrow.objectmatrix.position;
			packet.arrowRotation = arrow.modelmatrix.rotation;

			retval = send(sock, (char*)&packet, sizeof(packet), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				return 1;
			}

			retval = recv(sock, (char*)&inPacket, sizeof(inPacket), MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				return 1;
			}
			other_x_angle = inPacket.x_angle;
			other_y_angle = inPacket.y_angle;
			otherArrow.objectmatrix.position = inPacket.arrowPosition;
			otherArrow.modelmatrix.rotation = inPacket.arrowRotation;
			wind_dir = inPacket.wind_dir;
			wind_speed = inPacket.wind_speed;

			if (stage != inPacket.stage)
			{
				myWin = LOBYTE(inPacket.winCount);
				otherWin = HIBYTE(inPacket.winCount);
				t = 0;
				v = 0;
				y_angle = 0;
				x_angle = 0;
				camera_x = bow.objectmatrix.position.x + 0.2;
				camera_y = bow.objectmatrix.position.y + 0.2;
				camera_z = bow.objectmatrix.position.z + 0.6;
				arrow_x = 0.07;
				arrow_y = 0;
				arrow_z = 0.5;
				arrow_on = false;
				score_on = false;
				arrow.objectmatrix.position = initPacket.player1Pos;

				arrow.modelmatrix.rotation.x = 0;
				arrow.modelmatrix.rotation.y = 0;
				arrow.modelmatrix.rotation.z = 0;

				pass = false;
			}

			stage = inPacket.stage;
			myScore = LOBYTE(inPacket.total_score);
			otherScore = HIBYTE(inPacket.total_score);


			for (int i = 0; i < circleheight * circlewidth; ++i)
			{
				if (inPacket.circleState[i] == 2)
				{
					if (sqrt(pow(initPacket.circleCenter[i].x - arrow.objectmatrix.position.x, 2.0) + pow(initPacket.circleCenter[i].y - arrow.objectmatrix.position.y, 2.0)) <= (1.0))
					{
						particle_on = true;
						hitPos = initPacket.circleCenter[i];
					}
				}
			}
		}
	}
	return 0;
}

void main(int argc, char* argv[])
{
	if (argc > 1) SERVERIP = argv[1]; // default local
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	width = 800;
	height = 800;

	FILE* cubemap = fopen("texture/cube.obj", "rb");
	FILE* arrowfile = fopen("texture/arrow_face_no.obj", "rb");
	FILE* arrowuv = fopen("texture/arrowtest.txt", "rb");
	FILE* bowvertex = fopen("texture/bow_vertex.obj", "rb");
	FILE* bowuv = fopen("texture/bow_uvdata.obj", "rb");
	FILE* cube = fopen("texture/cube.obj", "rb");

	srand((unsigned)time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1600, 850);
	glutCreateWindow("test");
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW Initialized\n";
	}
	glEnable(GL_DEPTH_TEST);

	background.Readobj(cubemap);
	arrow.Readobj(arrowfile);
	arrow.Readuv(arrowuv);
	bow.Readobj(bowvertex);
	bow.Readuv(bowuv);

	otherArrow.Readobj(arrowfile);
	otherArrow.Readuv(arrowuv);
	otherBow.Readobj(bowvertex);
	otherBow.Readuv(bowuv);

	for (int i = 0; i < CUBE_SIZE; ++i)
	{
		paticle[i].Readobj(cube);
	}
	
	InitShader();
	InitBuffer();
	InitTexture();

	for (int i = 0; i < SNOW_SIZE; i++)
	{
		snow[i].modelmatrix.scale = glm::vec3(1.0, 1.0, 1.0);
		snow[i].modelmatrix.position = glm::vec3((float)((float)(rand() % snowSize) /snowSizeDiv) - snowStartX, snowStartY, (float)((float)(rand() % snowSize) / snowSizeDiv));
	}

	for (int i = 0; i < GRASS_SIZE; i++)
	{
		grass[i].modelmatrix.scale = glm::vec3(8.0, 1.0, 1.0);
		grass[i].modelmatrix.position = glm::vec3((float)((float)(rand() % snowSize) / snowSizeDiv) - snowStartX, snowStartY-95.0, (float)((float)(rand() % snowSize) / snowSizeDiv));
	}

	for (int i = 0; i < CUBE_SIZE; i++)
	{
		paticle[i].modelmatrix.scale = glm::vec3(0.1, 0.1, 0.1);
		paticle[i].modelmatrix.position = glm::vec3(0.0, 0.0, 40.0);
		particle_way_x[i] = rand() % 360;
		particle_way_y[i] = rand() % 360;
	}

	arrow.modelmatrix.scale = glm::vec3(0.6, 0.6, 0.6);
	background.modelmatrix.scale = glm::vec3(100.0, 100.0, 100.0);
	background.modelmatrix.position = glm::vec3(0.0, 15.0, 45.0);
	otherArrow.modelmatrix.scale = glm::vec3(0.6, 0.6, 0.6);

	arrow.objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);
	otherArrow.objectmatrix.position = glm::vec3(0.0, 0.0, 0.0);
	bow.objectmatrix.position = glm::vec3(arrow.objectmatrix.position.x - 0.07, arrow.objectmatrix.position.y, 0.0);
	otherBow.objectmatrix.position = glm::vec3(otherArrow.objectmatrix.position.x - 0.07, otherArrow.objectmatrix.position.y, 0.0);

	for (int i = 0; i < 10; i++)
	{
		circle[i].modelmatrix.position.z += 90.01 + 0.001 * i;
		circle[i].modelmatrix.scale = glm::vec3(i * 0.1 + 0.1, i * 0.1 + 0.1, 1.0);
	}

	line.modelmatrix.scale = glm::vec3(1.0, 1.0, 1.0);
	board.modelmatrix.scale = glm::vec3(1.0, 1.0, 1.0);
	board.modelmatrix.rotation.y = 180.0;
	board.modelmatrix.position.z = -0.5;

	Timer(0);

	glutDisplayFunc(drawScene);
	glutKeyboardFunc(Keyborad);
	glutKeyboardUpFunc(Keyborad_up);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMouseWheelFunc(mouseWheel);
	glutMainLoop();
}

int make_vertexShaders()
{
	vertexsource = filetobuf("vertex-project.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexsource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return false;
	}
}

int make_fragmentShaders()
{
	fragmentsource = filetobuf("fg-project.glsl");
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentsource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return false;
	}
}

GLuint make_shaderProgram()
{
	GLuint ShaderProgramID;
	ShaderProgramID = glCreateProgram();
	glAttachShader(ShaderProgramID, vertexShader);
	glAttachShader(ShaderProgramID, fragmentShader);
	glLinkProgram(ShaderProgramID);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(ShaderProgramID);
	return ShaderProgramID;
}

GLvoid drawScene()
{
	glViewport(0, 0, 800, 800);
	GLfloat rColor = 0.0f, gColor = 0.0f, bColor = 0.0f;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(s_program);
	glPolygonMode(GL_FRONT, Mode);

	if (main_loading == false)
	{
		cameratransform = glm::mat4(1.0f);
		cameratransform = glm::rotate(cameratransform, (float)glm::radians(x_angle), glm::vec3(1.0, 0.0, 0.0));
		cameratransform = glm::rotate(cameratransform, (float)glm::radians(y_angle + 180.0), glm::vec3(0.0, 1.0, 0.0));
		cameratransform = glm::translate(cameratransform, glm::vec3(-camera_x, -camera_y, -camera_z));
		unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(cameratransform));

		glm::mat4 perspect = glm::mat4(1.0f);
		perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));

		int cameraPosLocation = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation, 1, glm::value_ptr(cameraPos));
		int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation, x_1, y_1, z_1);
		int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		if (left_button == true && arrow_on == false)
		{
			line.Draw(s_program);
		}

		for (int i = 0; i < circleheight*circlewidth; ++i) {
			for (int j = 0; j < 10; ++j) {
				if(inPacket.circleState[i]) 
					circles[i][j].Draw(s_program);
			}
		}


		background.Draw(s_program, stage);

		arrow.Draw(s_program);
		bow.Draw(s_program);

		otherArrow.Draw(s_program);
		otherBow.Draw(s_program);

		if (stage == 1)
		{
			for (int i = 0; i < GRASS_SIZE; i++)
			{
				grass[i].Draw(s_program);
			}
		}

		if (stage == 3)
		{
			for (int i = 0; i < SNOW_SIZE; i++)
			{
				snow[i].Draw(s_program);
			}
		}

		if (particle_on)
		{
			for (int i = 0; i < CUBE_SIZE; ++i)
			{
				paticle[i].Draw(s_program);
			}
		}


		glViewport(0, 700, 100, 100);
		cameratransform1 = glm::mat4(1.0f);
		cameratransform1 = glm::rotate(cameratransform1, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform1 = glm::rotate(cameratransform1, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation1 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation1, 1, GL_FALSE, glm::value_ptr(cameratransform1));

		glm::mat4 perspect1 = glm::mat4(1.0f);
		perspect1 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect1 = glm::translate(perspect1, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation1 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation1, 1, GL_FALSE, glm::value_ptr(perspect1));

		int cameraPosLocation1 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation1, 1, glm::value_ptr(cameraPos));
		int lightPosLocation1 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation1, x_1, y_1, z_1);
		int lightColorLocation1 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation1, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw(s_program);

		glViewport(100, 700, 100, 100);
		cameratransform2 = glm::mat4(1.0f);
		cameratransform2 = glm::rotate(cameratransform2, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform2 = glm::rotate(cameratransform2, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation2 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation2, 1, GL_FALSE, glm::value_ptr(cameratransform2));

		glm::mat4 perspect2 = glm::mat4(1.0f);
		perspect2 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect2 = glm::translate(perspect2, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation2 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation2, 1, GL_FALSE, glm::value_ptr(perspect2));

		int cameraPosLocation2 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation2, 1, glm::value_ptr(cameraPos));
		int lightPosLocation2 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation2, x_1, y_1, z_1);
		int lightColorLocation2 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation1, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw1(s_program, myScore/10);

		glViewport(200, 700, 100, 100);
		cameratransform3 = glm::mat4(1.0f);
		cameratransform3 = glm::rotate(cameratransform3, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform3 = glm::rotate(cameratransform3, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation3 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation3, 1, GL_FALSE, glm::value_ptr(cameratransform3));

		glm::mat4 perspect3 = glm::mat4(1.0f);
		perspect3 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect3 = glm::translate(perspect3, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation3 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation3, 1, GL_FALSE, glm::value_ptr(perspect3));

		int cameraPosLocation3 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation3, 1, glm::value_ptr(cameraPos));
		int lightPosLocation3 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation3, x_1, y_1, z_1);
		int lightColorLocation3 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation3, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw2(s_program, myScore % 10);

		switch (wind_dir) {
		case 1:
			wind_angle_z = -90;
			break;
		case 2:
			wind_angle_z = -45;
			break;
		case 3:
			wind_angle_z = -0;
			break;
		case 4:
			wind_angle_z = 45;
			break;
		case 5:
			wind_angle_z = 90;
			break;
		case 6:
			wind_angle_z = 135;
			break;
		case 7:
			wind_angle_z = 180;
			break;
		case 8:
			wind_angle_z = 225;
			break;
		}
		glViewport(700, 700, 100, 100);
		cameratransform4 = glm::mat4(1.0f);
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(wind_angle_z), glm::vec3(0.0, 0.0, 1.0));
		unsigned int cameraLocation4 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation4, 1, GL_FALSE, glm::value_ptr(cameratransform4));


		glm::mat4 perspect4 = glm::mat4(1.0f);
		perspect4 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect4 = glm::translate(perspect4, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation4 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation4, 1, GL_FALSE, glm::value_ptr(perspect4));

		int cameraPosLocation4 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation4, 1, glm::value_ptr(cameraPos));
		int lightPosLocation4 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation4, x_1, y_1, z_1);
		int lightColorLocation4 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation4, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw3(s_program, wind_dir);

		glViewport(700, 600, 100, 100);
		cameratransform5 = glm::mat4(1.0f);
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation5 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation5, 1, GL_FALSE, glm::value_ptr(cameratransform5));

		glm::mat4 perspect5 = glm::mat4(1.0f);
		perspect5 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect5 = glm::translate(perspect5, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation5 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation5, 1, GL_FALSE, glm::value_ptr(perspect5));

		int cameraPosLocation5 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation5, 1, glm::value_ptr(cameraPos));
		int lightPosLocation5 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation5, x_1, y_1, z_1);
		int lightColorLocation5 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation5, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw4(s_program, wind_speed);

		glViewport(150, 800, 400, 50);
		cameratransform7 = glm::mat4(1.0f);
		cameratransform7 = glm::rotate(cameratransform7, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform7 = glm::rotate(cameratransform7, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation7 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation7, 1, GL_FALSE, glm::value_ptr(cameratransform7));

		glm::mat4 perspect7 = glm::mat4(1.0f);
		perspect7 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect7 = glm::translate(perspect7, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation7 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation7, 1, GL_FALSE, glm::value_ptr(perspect7));

		int cameraPosLocation7 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation7, 1, glm::value_ptr(cameraPos));
		int lightPosLocation7= glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation7, x_1, y_1, z_1);
		int lightColorLocation7 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation7, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw7(s_program);

		glViewport(650, 800, 50, 50);
		cameratransform9 = glm::mat4(1.0f);
		cameratransform9 = glm::rotate(cameratransform9, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform9 = glm::rotate(cameratransform9, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation9 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation9, 1, GL_FALSE, glm::value_ptr(cameratransform9));

		glm::mat4 perspect9 = glm::mat4(1.0f);
		perspect9 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect9 = glm::translate(perspect9, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation9 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation9, 1, GL_FALSE, glm::value_ptr(perspect9));

		int cameraPosLocation9 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation9, 1, glm::value_ptr(cameraPos));
		int lightPosLocation9 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation9, x_1, y_1, z_1);
		int lightColorLocation9 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation9, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw1(s_program,myWin);

		glViewport(550, 775, 100, 100);
		cameratransform11 = glm::mat4(1.0f);
		cameratransform11 = glm::rotate(cameratransform11, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform11 = glm::rotate(cameratransform11, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation11 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation11, 1, GL_FALSE, glm::value_ptr(cameratransform11));

		glm::mat4 perspect11 = glm::mat4(1.0f);
		perspect11 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect11 = glm::translate(perspect11, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation11 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation11, 1, GL_FALSE, glm::value_ptr(perspect11));

		int cameraPosLocation11 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation11, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation11 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation11, x_1, y_1, z_1);
		int lightColorLocation11 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation11, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw8(s_program);
	}
	else if (main_loading)
	{
		glViewport(-100, -100, 1000, 1000);
		cameratransform5 = glm::mat4(1.0f);
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation5 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation5, 1, GL_FALSE, glm::value_ptr(cameratransform5));

		glm::mat4 perspect5 = glm::mat4(1.0f);
		perspect5 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect5 = glm::translate(perspect5, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation5 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation5, 1, GL_FALSE, glm::value_ptr(perspect5));

		int cameraPosLocation5 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation5, 1, glm::value_ptr(cameraPos));
		int lightPosLocation5 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation5, x_1, y_1, z_1);
		int lightColorLocation5 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation5, Light_R, Light_G, Light_B);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw5(s_program);
	}

	//상대화면
	glViewport(800, 0, 800, 800);
	if (main_loading == false)
	{
		cameratransform = glm::mat4(1.0f);
		cameratransform = glm::rotate(cameratransform, (float)glm::radians(other_x_angle), glm::vec3(1.0, 0.0, 0.0));
		cameratransform = glm::rotate(cameratransform, (float)glm::radians(other_y_angle + 180.0), glm::vec3(0.0, 1.0, 0.0));
		cameratransform = glm::translate(cameratransform, glm::vec3(-otherCamera_x, -otherCamera_y, -otherCamera_z));
		unsigned int cameraLocation = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation, 1, GL_FALSE, glm::value_ptr(cameratransform));

		glm::mat4 perspect = glm::mat4(1.0f);
		perspect = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect = glm::translate(perspect, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(perspect));

		int cameraPosLocation = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation, x_1, y_1, z_1);
		int lightColorLocation = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		for (int i = 0; i < circleheight * circlewidth; ++i) {
			for (int j = 0; j < 10; ++j) {
				if (inPacket.circleState[i]) 
					circles[i][j].Draw(s_program);
			}
		}

		background.Draw(s_program, stage);

		arrow.Draw(s_program);
		bow.Draw(s_program);

		otherArrow.Draw(s_program);
		otherBow.Draw(s_program);

		if (stage == 1)
		{
			for (int i = 0; i < GRASS_SIZE; i++)
			{
				grass[i].Draw(s_program);
			}
		}

		if (stage == 3)
		{
			for (int i = 0; i < SNOW_SIZE; i++)
			{
				snow[i].Draw(s_program);
			}
		}

		if (particle_on)
		{
			for (int i = 0; i < CUBE_SIZE; ++i)
			{
				paticle[i].Draw(s_program);
			}
		}


		glViewport(800, 700, 100, 100);
		cameratransform1 = glm::mat4(1.0f);
		cameratransform1 = glm::rotate(cameratransform1, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform1 = glm::rotate(cameratransform1, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation1 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation1, 1, GL_FALSE, glm::value_ptr(cameratransform1));

		glm::mat4 perspect1 = glm::mat4(1.0f);
		perspect1 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect1 = glm::translate(perspect1, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation1 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation1, 1, GL_FALSE, glm::value_ptr(perspect1));

		int cameraPosLocation1 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation1, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation1 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation1, x_1, y_1, z_1);
		int lightColorLocation1 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation1, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw(s_program);

		glViewport(900, 700, 100, 100);
		cameratransform2 = glm::mat4(1.0f);
		cameratransform2 = glm::rotate(cameratransform2, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform2 = glm::rotate(cameratransform2, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation2 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation2, 1, GL_FALSE, glm::value_ptr(cameratransform2));

		glm::mat4 perspect2 = glm::mat4(1.0f);
		perspect2 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect2 = glm::translate(perspect2, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation2 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation2, 1, GL_FALSE, glm::value_ptr(perspect2));

		int cameraPosLocation2 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation2, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation2 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation2, x_1, y_1, z_1);
		int lightColorLocation2 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation1, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw1(s_program, otherScore/10);

		glViewport(1000, 700, 100, 100);
		cameratransform3 = glm::mat4(1.0f);
		cameratransform3 = glm::rotate(cameratransform3, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform3 = glm::rotate(cameratransform3, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation3 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation3, 1, GL_FALSE, glm::value_ptr(cameratransform3));

		glm::mat4 perspect3 = glm::mat4(1.0f);
		perspect3 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect3 = glm::translate(perspect3, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation3 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation3, 1, GL_FALSE, glm::value_ptr(perspect3));

		int cameraPosLocation3 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation3, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation3 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation3, x_1, y_1, z_1);
		int lightColorLocation3 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation3, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw2(s_program, otherScore % 10);

		switch (wind_dir) {
		case 1:
			wind_angle_z = -90;
			break;
		case 2:
			wind_angle_z = -45;
			break;
		case 3:
			wind_angle_z = -0;
			break;
		case 4:
			wind_angle_z = 45;
			break;
		case 5:
			wind_angle_z = 90;
			break;
		case 6:
			wind_angle_z = 135;
			break;
		case 7:
			wind_angle_z = 180;
			break;
		case 8:
			wind_angle_z = 225;
			break;
		}
		glViewport(1500, 700, 100, 100);
		cameratransform4 = glm::mat4(1.0f);
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		cameratransform4 = glm::rotate(cameratransform4, (float)glm::radians(wind_angle_z), glm::vec3(0.0, 0.0, 1.0));
		unsigned int cameraLocation4 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation4, 1, GL_FALSE, glm::value_ptr(cameratransform4));


		glm::mat4 perspect4 = glm::mat4(1.0f);
		perspect4 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect4 = glm::translate(perspect4, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation4 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation4, 1, GL_FALSE, glm::value_ptr(perspect4));

		int cameraPosLocation4 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation4, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation4 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation4, x_1, y_1, z_1);
		int lightColorLocation4 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation4, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw3(s_program, wind_dir);

		glViewport(1500, 600, 100, 100);
		cameratransform5 = glm::mat4(1.0f);
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation5 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation5, 1, GL_FALSE, glm::value_ptr(cameratransform5));

		glm::mat4 perspect5 = glm::mat4(1.0f);
		perspect5 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect5 = glm::translate(perspect5, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation5 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation5, 1, GL_FALSE, glm::value_ptr(perspect5));

		int cameraPosLocation5 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation5, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation5 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation5, x_1, y_1, z_1);
		int lightColorLocation5 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation5, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw4(s_program, wind_speed);

		glViewport(950, 800, 400, 50);
		cameratransform8 = glm::mat4(1.0f);
		cameratransform8 = glm::rotate(cameratransform8, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform8 = glm::rotate(cameratransform8, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation8 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation8, 1, GL_FALSE, glm::value_ptr(cameratransform8));

		glm::mat4 perspect8 = glm::mat4(1.0f);
		perspect8 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect8 = glm::translate(perspect8, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation8 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation8, 1, GL_FALSE, glm::value_ptr(perspect8));

		int cameraPosLocation8 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation8, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation8 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation8, x_1, y_1, z_1);
		int lightColorLocation8 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation8, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw6(s_program);

		glViewport(1450, 800, 50, 50);
		cameratransform10 = glm::mat4(1.0f);
		cameratransform10 = glm::rotate(cameratransform10, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform10 = glm::rotate(cameratransform10, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation10 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation10, 1, GL_FALSE, glm::value_ptr(cameratransform10));

		glm::mat4 perspect10 = glm::mat4(1.0f);
		perspect10 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect10 = glm::translate(perspect10, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation10 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation10, 1, GL_FALSE, glm::value_ptr(perspect10));

		int cameraPosLocation10 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation10, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation10 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation10, x_1, y_1, z_1);
		int lightColorLocation10 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation10, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw1(s_program, otherWin);

		glViewport(1350, 775, 100, 100);
		cameratransform11 = glm::mat4(1.0f);
		cameratransform11 = glm::rotate(cameratransform11, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform11 = glm::rotate(cameratransform11, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation11 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation11, 1, GL_FALSE, glm::value_ptr(cameratransform11));

		glm::mat4 perspect11 = glm::mat4(1.0f);
		perspect11 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect11 = glm::translate(perspect11, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation11 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation11, 1, GL_FALSE, glm::value_ptr(perspect11));

		int cameraPosLocation11 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation11, 1, glm::value_ptr(otherCameraPos));
		int lightPosLocation11 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation11, x_1, y_1, z_1);
		int lightColorLocation11 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation11, Light_R, Light_G, Light_B);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw8(s_program);
	}
	else if (main_loading)
	{
		glViewport(700, -100, 1000, 1000);
		cameratransform5 = glm::mat4(1.0f);
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0), glm::vec3(1.0, 0.0, 0.0));
		cameratransform5 = glm::rotate(cameratransform5, (float)glm::radians(0.0 + 180.0), glm::vec3(0.0, 1.0, 0.0));
		unsigned int cameraLocation5 = glGetUniformLocation(s_program, "cameraTransform");
		glUniformMatrix4fv(cameraLocation5, 1, GL_FALSE, glm::value_ptr(cameratransform5));

		glm::mat4 perspect5 = glm::mat4(1.0f);
		perspect5 = glm::perspective(glm::radians(fovy), (float)width / (float)height, near_1, far_1);
		perspect5 = glm::translate(perspect5, glm::vec3(0.0, 0.0, persfect_z));
		unsigned int projectionLocation5 = glGetUniformLocation(s_program, "projectionTransform");
		glUniformMatrix4fv(projectionLocation5, 1, GL_FALSE, glm::value_ptr(perspect5));

		int cameraPosLocation5 = glGetUniformLocation(s_program, "cameraPos");
		glUniform3fv(cameraPosLocation5, 1, glm::value_ptr(cameraPos));
		int lightPosLocation5 = glGetUniformLocation(s_program, "lightPos");
		glUniform3f(lightPosLocation5, x_1, y_1, z_1);
		int lightColorLocation5 = glGetUniformLocation(s_program, "lightColor");
		glUniform3f(lightColorLocation5, Light_R, Light_G, Light_B);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glActiveTexture(GL_TEXTURE0);

		board.Draw5(s_program);
	}

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

char* filetobuf(const char* file)
{
	char* buf;
	FILE* fptr = fopen(file, "rb");
	if (!fptr)
		return NULL;
	fseek(fptr, 0, SEEK_END);
	long length = ftell(fptr);
	buf = (char*)malloc(length + 1);
	fseek(fptr, 0, SEEK_SET);
	fread(buf, length, 1, fptr);
	fclose(fptr);
	buf[length] = 0;
	return buf;
}

void InitBuffer()
{
	background.Initialize();
	arrow.Initialize();
	line.Initialize();
	board.Initialize();
	bow.Initialize();

	otherArrow.Initialize();
	otherBow.Initialize();

	for (int i = 0; i < 10; i++)
	{
		circle[i].Initialize();
	}

	//과녁 위치
	circle[0].Update(1.0, 1.0, 0.7);
	circle[1].Update(1.0, 1.0, 0.0);
	circle[2].Update(1.0, 0.2, 0.2);
	circle[3].Update(1.0, 0.0, 0.0);
	circle[4].Update(0.2, 0.2, 1.0);
	circle[5].Update(0.0, 0.0, 1.0);
	circle[6].Update(0.2, 0.2, 0.2);
	circle[7].Update(0.4, 0.4, 0.4);
	circle[8].Update(0.6, 0.6, 0.6);
	circle[9].Update(0.8, 0.8, 0.8);

	for (int i = 0; i < SNOW_SIZE; i++)
	{
		snow[i].Initialize();
	}

	for (int i = 0; i < GRASS_SIZE; i++)
	{
		grass[i].Initialize();
	}

	for (int i = 0; i < CUBE_SIZE; ++i)
	{
		paticle[i].Initialize();
	}
}

void InitTexture()
{
	int widthImage, heightImage, numberOfChannel;
	int widthImage_snow, heightImage_snow, numberOfChannel_snow;
	int widthImage_grass, heightImage_grass, numberOfChannel_grass;
	int widthImage_arctic, heightImage_arctic, numberOfChannel_arctic;
	int widthImage_ocean, heightImage_ocean, numberOfChannel_ocean;
	int widthImage_arrow, heightImage_arrow, numberOfChannel_arrow;
	int widthImage_white, heightImage_white, numberOfChannel_white;
	int widthImage_field, heightImage_field, numberOfChannel_field;
	int widthImage_score, heightImage_score, numberOfChannel_score;
	int widthImage_number, heightImage_number, numberOfChannel_number;
	int widthImage_direction, heightImage_direction, numberOfChannel_direction;
	int widthImage_loading, heightImage_loading, numberOfChannel_loading;
	int widthImage_cube, heightImage_cube, numberOfChannel_cube;

	int widthImage_enemydisplay, heightImage_enemydisplay, numberOfChannel_enemydisplay;
	int widthImage_mydisplay, heightImage_mydisplay, numberOfChannel_mydisplay;

	int widthImage_win_count, heightImage_win_count, numberOfChannel_win_count;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* sky1 = stbi_load("texture/sky1.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky2 = stbi_load("texture/sky2.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky3 = stbi_load("texture/sky3.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky4 = stbi_load("texture/sky4.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky5 = stbi_load("texture/sky5.png", &widthImage, &heightImage, &numberOfChannel, 0);
	unsigned char* sky6 = stbi_load("texture/sky6.png", &widthImage, &heightImage, &numberOfChannel, 0);

	unsigned char* snow = stbi_load("texture/snow.png", &widthImage_snow, &heightImage_snow, &numberOfChannel_snow, 0);
	unsigned char* grass = stbi_load("texture/grass.png", &widthImage_grass, &heightImage_grass, &numberOfChannel_grass, 0);

	unsigned char* arctic1 = stbi_load("texture/arctic1.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic2 = stbi_load("texture/arctic2.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic3 = stbi_load("texture/arctic3.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic4 = stbi_load("texture/arctic4.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic5 = stbi_load("texture/arctic5.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);
	unsigned char* arctic6 = stbi_load("texture/arctic6.png", &widthImage_arctic, &heightImage_arctic, &numberOfChannel_arctic, 0);

	unsigned char* ocean1 = stbi_load("texture/ocean1.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean2 = stbi_load("texture/ocean2.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean3 = stbi_load("texture/ocean3.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean4 = stbi_load("texture/ocean4.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean5 = stbi_load("texture/ocean5.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);
	unsigned char* ocean6 = stbi_load("texture/ocean6.png", &widthImage_ocean, &heightImage_ocean, &numberOfChannel_ocean, 0);

	unsigned char* arrow = stbi_load("texture/arrow_basebolor.png", &widthImage_arrow, &heightImage_arrow, &numberOfChannel_arrow, 0);

	unsigned char* white = stbi_load("texture/white.png", &widthImage_white, &heightImage_white, &numberOfChannel_white, 0);

	unsigned char* field1 = stbi_load("texture/field1.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field2 = stbi_load("texture/field2.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field3 = stbi_load("texture/field3.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field4 = stbi_load("texture/field4.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field5 = stbi_load("texture/field5.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);
	unsigned char* field6 = stbi_load("texture/field6.png", &widthImage_field, &heightImage_field, &numberOfChannel_field, 0);

	unsigned char* score = stbi_load("texture/score.png", &widthImage_score, &heightImage_score, &numberOfChannel_score, 0);

	unsigned char* number0 = stbi_load("texture/0.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number1 = stbi_load("texture/1.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number2 = stbi_load("texture/2.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number3 = stbi_load("texture/3.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number4 = stbi_load("texture/4.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number5 = stbi_load("texture/5.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number6 = stbi_load("texture/6.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number7 = stbi_load("texture/7.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number8 = stbi_load("texture/8.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);
	unsigned char* number9 = stbi_load("texture/9.png", &widthImage_number, &heightImage_number, &numberOfChannel_number, 0);

	unsigned char* direction = stbi_load("texture/direction.png", &widthImage_direction, &heightImage_direction, &numberOfChannel_direction, 0);

	unsigned char* loading = stbi_load("texture/loading.png", &widthImage_loading, &heightImage_loading, &numberOfChannel_loading, 0);

	unsigned char* cube = stbi_load("texture/cube.png", &widthImage_cube, &heightImage_cube, &numberOfChannel_cube, 0);

	unsigned char* enemydisplay = stbi_load("texture/enemydisplay.png", &widthImage_enemydisplay, &heightImage_enemydisplay, &numberOfChannel_enemydisplay, 0);
	unsigned char* mydisplay = stbi_load("texture/mydisplay.png", &widthImage_mydisplay, &heightImage_mydisplay, &numberOfChannel_mydisplay, 0);
	unsigned char* win_count = stbi_load("texture/win_count.png", &widthImage_win_count, &heightImage_win_count, &numberOfChannel_win_count, 0);


	glGenTextures(45, texture);

	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky1);
	stbi_image_free(sky1);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky2);
	stbi_image_free(sky2);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky3);
	stbi_image_free(sky3);

	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky4);
	stbi_image_free(sky4);

	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky5);
	stbi_image_free(sky5);

	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage, heightImage, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky6);
	stbi_image_free(sky6);



	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_snow, heightImage_snow, 0, GL_RGBA, GL_UNSIGNED_BYTE, snow);
	stbi_image_free(snow);

	glBindTexture(GL_TEXTURE_2D, texture[7]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_grass, heightImage_grass, 0, GL_RGBA, GL_UNSIGNED_BYTE, grass);
	stbi_image_free(grass);


	glBindTexture(GL_TEXTURE_2D, texture[8]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic1);
	stbi_image_free(arctic1);

	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic2);
	stbi_image_free(arctic2);

	glBindTexture(GL_TEXTURE_2D, texture[10]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic3);
	stbi_image_free(arctic3);

	glBindTexture(GL_TEXTURE_2D, texture[11]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic4);
	stbi_image_free(arctic4);

	glBindTexture(GL_TEXTURE_2D, texture[12]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic5);
	stbi_image_free(arctic5);

	glBindTexture(GL_TEXTURE_2D, texture[13]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arctic, heightImage_arctic, 0, GL_RGBA, GL_UNSIGNED_BYTE, arctic6);
	stbi_image_free(arctic6);



	glBindTexture(GL_TEXTURE_2D, texture[14]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean1);
	stbi_image_free(ocean1);

	glBindTexture(GL_TEXTURE_2D, texture[15]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean2);
	stbi_image_free(ocean2);

	glBindTexture(GL_TEXTURE_2D, texture[16]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean3);
	stbi_image_free(ocean3);

	glBindTexture(GL_TEXTURE_2D, texture[17]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean4);
	stbi_image_free(ocean4);

	glBindTexture(GL_TEXTURE_2D, texture[18]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean5);
	stbi_image_free(ocean5);

	glBindTexture(GL_TEXTURE_2D, texture[19]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_ocean, heightImage_ocean, 0, GL_RGBA, GL_UNSIGNED_BYTE, ocean6);
	stbi_image_free(ocean6);



	glBindTexture(GL_TEXTURE_2D, texture[20]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_arrow, heightImage_arrow, 0, GL_RGBA, GL_UNSIGNED_BYTE, arrow);
	stbi_image_free(arrow);

	glBindTexture(GL_TEXTURE_2D, texture[21]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_white, heightImage_white, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
	stbi_image_free(white);

	glBindTexture(GL_TEXTURE_2D, texture[22]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field1);
	stbi_image_free(field1);

	glBindTexture(GL_TEXTURE_2D, texture[23]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field2);
	stbi_image_free(field2);

	glBindTexture(GL_TEXTURE_2D, texture[24]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field3);
	stbi_image_free(field3);

	glBindTexture(GL_TEXTURE_2D, texture[25]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field4);
	stbi_image_free(field4);

	glBindTexture(GL_TEXTURE_2D, texture[26]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field5);
	stbi_image_free(field5);

	glBindTexture(GL_TEXTURE_2D, texture[27]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_field, heightImage_field, 0, GL_RGBA, GL_UNSIGNED_BYTE, field6);
	stbi_image_free(field6);

	glBindTexture(GL_TEXTURE_2D, texture[28]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_score, heightImage_score, 0, GL_RGBA, GL_UNSIGNED_BYTE, score);
	stbi_image_free(score);

	glBindTexture(GL_TEXTURE_2D, texture[29]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number0);
	stbi_image_free(number0);

	glBindTexture(GL_TEXTURE_2D, texture[30]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number1);
	stbi_image_free(number1);

	glBindTexture(GL_TEXTURE_2D, texture[31]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number2);
	stbi_image_free(number2);

	glBindTexture(GL_TEXTURE_2D, texture[32]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number3);
	stbi_image_free(number3);

	glBindTexture(GL_TEXTURE_2D, texture[33]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number4);
	stbi_image_free(number4);

	glBindTexture(GL_TEXTURE_2D, texture[34]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number5);
	stbi_image_free(number5);

	glBindTexture(GL_TEXTURE_2D, texture[35]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number6);
	stbi_image_free(number6);

	glBindTexture(GL_TEXTURE_2D, texture[36]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number7);
	stbi_image_free(number7);

	glBindTexture(GL_TEXTURE_2D, texture[37]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number8);
	stbi_image_free(number8);

	glBindTexture(GL_TEXTURE_2D, texture[38]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_number, heightImage_number, 0, GL_RGBA, GL_UNSIGNED_BYTE, number9);
	stbi_image_free(number9);

	glBindTexture(GL_TEXTURE_2D, texture[39]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_direction, heightImage_direction, 0, GL_RGBA, GL_UNSIGNED_BYTE, direction);
	stbi_image_free(direction);

	glBindTexture(GL_TEXTURE_2D, texture[40]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_loading, heightImage_loading, 0, GL_RGBA, GL_UNSIGNED_BYTE, loading);
	stbi_image_free(loading);

	glBindTexture(GL_TEXTURE_2D, texture[41]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_cube, heightImage_cube, 0, GL_BGRA, GL_UNSIGNED_BYTE, cube);
	stbi_image_free(cube);

	glBindTexture(GL_TEXTURE_2D, texture[42]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_enemydisplay, heightImage_enemydisplay, 0, GL_BGRA, GL_UNSIGNED_BYTE, enemydisplay);
	stbi_image_free(enemydisplay);

	glBindTexture(GL_TEXTURE_2D, texture[43]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_mydisplay, heightImage_mydisplay, 0, GL_BGRA, GL_UNSIGNED_BYTE, mydisplay);
	stbi_image_free(mydisplay);

	glBindTexture(GL_TEXTURE_2D, texture[44]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, widthImage_win_count, heightImage_win_count, 0, GL_BGRA, GL_UNSIGNED_BYTE, win_count);
	stbi_image_free(win_count);

	glUseProgram(s_program);
	int tLocation = glGetUniformLocation(s_program, "outTexture");
	glUniform1i(tLocation, 0);
}

void InitShader()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	s_program = glCreateProgram();
	glAttachShader(s_program, vertexShader);
	glAttachShader(s_program, fragmentShader);
	glLinkProgram(s_program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glUseProgram(s_program);
}

void Convert_xy(int x, int y)
{
	float w = width;
	float h = height;
	ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void Timer(int value)
{
	for (int i = 0; i < SNOW_SIZE; i++)
	{
		snow[i].modelmatrix.position.y -= snow[i].speed;
		if (snow[i].modelmatrix.position.y <= -15.0)
		{
			snow[i].modelmatrix.position.y = 65.0;
		}
	}

	if (arrow_on)
	{
		if (arrow_z < 40.0 && arrow_y > -10.0)
		{
			switch (wind_dir) {
			case 0:
				wind_x += 0;
				wind_y += 0;
				wind_z += 0;
				break;
			case 1:
				wind_x += 0;
				wind_y += (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_z += 0;
				break;
			case 2:
				wind_x -= (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_y += (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_z += 0;
				break;
			case 3:
				wind_x -= (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_y += 0;
				wind_z += 0;
				break;
			case 4:
				wind_x -= (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_y -= (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_z += 0;
				break;
			case 5:
				wind_x -= 0;
				wind_y -= (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_z += 0;
				break;
			case 6:
				wind_x += (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_y -= (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_z += 0;
				break;
			case 7:
				wind_x += (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_y += 0;
				wind_z += 0;
				break;
			case 8:
				wind_x += (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_y += (0.0009 * 1.229 * pow(wind_speed, 2.0));
				wind_z += 0;
				break;
			default:
				break;
			}
			t += 0.01;
			arrow_z = v * cos(arrow_angle_y) * t;
			arrow_y = v * sin(arrow_angle_y) * t - (0.5 * 9.8 * t * t);
			if (arrow_angle_x != 0)
			{
				arrow_x = float(arrow_z / tan(arrow_angle_x));
			}
			else
			{
				arrow_x = 0;
			}
			arrow.objectmatrix.position = glm::vec3(arrow_x + wind_x + initPacket.player1Pos.x, arrow_y + wind_y, arrow_z + wind_z);
			arrow.modelmatrix.rotation.x = -atanf((arrow_y - pre_arrow_y) / (arrow_z - pre_arrow_z)) * (180.0 / PI);
			arrow.modelmatrix.rotation.y = atanf((arrow_x - pre_arrow_x) / (arrow_z - pre_arrow_z)) * (180.0 / PI);
			pre_arrow_x = arrow_x;
			pre_arrow_y = arrow_y;
			pre_arrow_z = arrow_z;

			if (camera_mode == 0)
			{
				camera_y = arrow_y + wind_y;
				camera_z = arrow_z + wind_z;
				camera_x = arrow_x + wind_x + initPacket.player1Pos.x;
			}
			else if (camera_mode == 1)
			{
				camera_z = 88;
				camera_y = 0;
				camera_x = 0;
				y_angle = 180;
			}
		}
	}

	if (particle_on)
	{
		if (particle_during == 1.0)
		{
			for (int i = 0; i < CUBE_SIZE; ++i)
			{
				paticle[i].modelmatrix.position.x = hitPos.x;
				paticle[i].modelmatrix.position.y = hitPos.y;
			}
		}
		particleSpeed += particleAcc;
		for (int i = 0; i < CUBE_SIZE; ++i)
		{
			paticle[i].modelmatrix.position.x += cos(particle_way_x[i] * PI / 180) * particleSpeed;
			paticle[i].modelmatrix.position.y += sin(particle_way_y[i] * PI / 180) * particleSpeed;

			paticle[i].modelmatrix.position.z -= particleSubZ;
		}

		particle_during -= 0.01;
		if (particle_during < 0)
		{
			particle_during = 1.0f;
			particle_on = false;
			particleSpeed = 0;
			for (int i = 0; i < CUBE_SIZE; ++i)
			{
				paticle[i].modelmatrix.position.x = 0;
				paticle[i].modelmatrix.position.y = 0;
				paticle[i].modelmatrix.position.z = 40;
			}
		}
	}

	if (left_button)
	{
		line.Update(v, y_angle, -x_angle * PI / 180.0 , arrow.objectmatrix.position.x);
	}

	if (left_button && arrow_on == false)
	{
		if (v < 40)
		{
			v += 0.1;
		}
		arrow.objectmatrix.position.z = -v * acc + arrowMoveZ;
		arrow.objectmatrix.position.x = initPacket.player1Pos.x;
		arrow_angle_y = -x_angle * PI / 180.0;
		arrow.modelmatrix.rotation.x = -atanf(arrow_angle_y) * (180.0 / PI);
		bow.modelmatrix.rotation.x = -atanf(arrow_angle_y) * (180.0 / PI);
	}
	else if (left_button == false && arrow_on == false)
	{
		v = 0;
	}

	number_1 = total_score % 10;
	number_10 = total_score / 10;


	key_check();
	glutTimerFunc(10, Timer, value);
	glutPostRedisplay();
}

void key_check()
{
	if (keybuffer['w'] == true || keybuffer['W'] == true)
	{
		camera_z += 0.1;
	}

	if (keybuffer['a'] == true || keybuffer['A'] == true)
	{
		camera_x += 0.1;
	}

	if (keybuffer['s'] == true || keybuffer['S'] == true)
	{
		camera_z -= 0.1;
	}

	if (keybuffer['d'] == true || keybuffer['D'] == true)
	{
		camera_x -= 0.1;
	}

	if (keybuffer['z'] == true || keybuffer['Z'] == true)
	{
		camera_y += 0.1;
	}

	if (keybuffer['x'] == true || keybuffer['X'] == true)
	{
		camera_y -= 0.1;
	}
}

GLvoid Keyborad(unsigned char key, int x, int y)
{
	if (GLUT_KEY_DOWN)
	{
		keybuffer[key] = true;
	}
	switch (key) {
	case 'o':
	case 'O':
		if (arrow_on == false && left_button)
		{
			wind_x = 0;
			wind_y = 0;
			wind_z = 0;
			arrow.objectmatrix.position = bow.objectmatrix.position;
			arrow.modelmatrix.rotation.x = 0;
			arrow.modelmatrix.rotation.y = 0;
			arrow.modelmatrix.rotation.z = 0;
			arrow_angle_y = -x_angle * PI / 180.0;
			if (y_angle < 0)
			{
				y_angle = 180 + y_angle;
			}
			if (y_angle > 0)
			{
				y_angle = 90 + y_angle;
			}
			arrow_angle_x = y_angle * PI / 180.0;
			arrow_on = true;
			y_angle = 0;
			x_angle = 0;
		}
		break;
	case '8':
		camera_mode = 0;
		break;
	case '9':
		camera_mode = 1;
		break;
	case 'm':
	case 'M':
		if (arrow_on == true)
		{
			t = 0;
			v = 0;
			y_angle = 0;
			x_angle = 0;
			camera_x = bow.objectmatrix.position.x + 0.2;
			camera_y = bow.objectmatrix.position.y + 0.2;
			camera_z = bow.objectmatrix.position.z + 0.6;
			arrow_x = 0.07;
			arrow_y = 0;
			arrow_z = 0.5;
			arrow_on = false;
			score_on = false;
			arrow.objectmatrix.position = initPacket.player1Pos;

			arrow.modelmatrix.rotation.x = 0;
			arrow.modelmatrix.rotation.y = 0;
			arrow.modelmatrix.rotation.z = 0;
			arrow_angle_y = -x_angle * PI / 180.0;
			if (y_angle < 0)
			{
				y_angle = 180 + y_angle;
			}
			if (y_angle > 0)
			{
				y_angle = 90 + y_angle;
			}
			arrow_angle_x = y_angle * PI / 180.0;
		}
		break;
	case 'N':
	case 'n':
		y_angle = 0;
		x_angle = 0;
		camera_x = bow.objectmatrix.position.x + 0.2;
		camera_y = bow.objectmatrix.position.y + 0.2;
		camera_z = bow.objectmatrix.position.z + 0.6;
		break;
	case 't':
	case 'T':
		if(main_loading)
		{
			main_loading = false;
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (sock == INVALID_SOCKET) err_quit("socket()");
			connectState=CreateSocket();
		}
		break;
	case 27: // Escape
		// 소켓 삭제 ------------
		closesocket(sock);
		WSACleanup();
		glutLeaveMainLoop();
	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid Keyborad_up(unsigned char key, int x, int y)
{
	if (GLUT_KEY_UP)
	{
		keybuffer[key] = false;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		ox = x;
		oy = y;
		x_angle = 0;
		y_angle = 0;
		left_button = true;
	}
	else
	{
		ox = 0;
		oy = 0;
		pre_x_angle = x_angle;
		pre_y_angle = y_angle;
		left_button = false;
	}
}

GLvoid Motion(int x, int y)
{
	if (left_button)
	{
		y_angle = x - ox;
		x_angle = y - oy;

		y_angle /= 8;
		x_angle /= 8;
		for (int i = 0; i < SNOW_SIZE; i++)
		{
			snow[i].modelmatrix.rotation.y = -y_angle;
		}
		for (int i = 0; i < GRASS_SIZE; i++)
		{
			grass[i].modelmatrix.rotation.y = -y_angle;
		}
	}
	glutPostRedisplay();
}

GLvoid mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		wheel_scale += dir * 0.1;
	}
	else if (dir < 0)
	{
		wheel_scale += dir * 0.1;
		if (wheel_scale < 0.1)
		{
			wheel_scale = 0.1;
		}
	}
	glutPostRedisplay();
}

bool CreateSocket()
{
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
	{
		err_quit("connect()");
		return false;
	}

	//데이터 연결을 성공했을 시에 초기데이터를 받는 곳
	DWORD wh = MAKEWORD(CIRCLENUMWIDTH, CIRCLENUMHEIGHT);
	retval = recv(sock, (char*)&wh, sizeof(wh), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("recv() wh");
		return true;
	}
	
	circlewidth = LOBYTE(wh);
	circleheight = HIBYTE(wh);
	retval = recv(sock, (char*)&initPacket, sizeof(initPacket), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("recv() InitPacket");
		return true;
	}

	circles = new Circle *[circlewidth * circleheight];
	for (int i = 0; i < circleheight * circlewidth; ++i)
	{
		circles[i] = new Circle[10];
	}

	for (int i = 0; i < circleheight * circlewidth; ++i) {
		for (int j = 0; j < 10; j++)
		{
			circles[i][j].Initialize();
			circles[i][j].objectmatrix.position = glm::vec3(0, 0, 0);
			circles[i][j].objectmatrix.rotation = glm::vec3(0, 0, 0);
			circles[i][j].objectmatrix.scale = glm::vec3(1, 1, 1);
			circles[i][j].modelmatrix.position = glm::vec3(0, 0, 0);
			circles[i][j].modelmatrix.rotation = glm::vec3(0, 0, 0);
			circles[i][j].modelmatrix.scale = glm::vec3(1, 1, 1);
		}
	}
	for (int i = 0; i < circleheight * circlewidth; ++i) {

		circles[i][0].Update(1.0, 1.0, 0.7);
		circles[i][1].Update(1.0, 1.0, 0.0);
		circles[i][2].Update(1.0, 0.2, 0.2);
		circles[i][3].Update(1.0, 0.0, 0.0);
		circles[i][4].Update(0.2, 0.2, 1.0);
		circles[i][5].Update(0.0, 0.0, 1.0);
		circles[i][6].Update(0.2, 0.2, 0.2);
		circles[i][7].Update(0.4, 0.4, 0.4);
		circles[i][8].Update(0.6, 0.6, 0.6);
		circles[i][9].Update(0.8, 0.8, 0.8);
	}

	for (int i = 0; i < circlewidth* circleheight; ++i) {
		for (int j = 0; j < 10; ++j) {
			circles[i][j].objectmatrix.position = initPacket.circleCenter[i];
			circles[i][j].objectmatrix.position.z += 0.001 * j;
			circles[i][j].objectmatrix.scale = glm::vec3(j * 0.1 + 0.1, j * 0.1 + 0.1, 1.0);
		}
	}
	
	arrow.objectmatrix.position= initPacket.player1Pos;
	otherArrow.objectmatrix.position= initPacket.player2Pos;
	bow.objectmatrix.position = glm::vec3(arrow.objectmatrix.position.x - 0.07, arrow.objectmatrix.position.y, 0.0);
	otherBow.objectmatrix.position = glm::vec3(otherArrow.objectmatrix.position.x - 0.07, otherArrow.objectmatrix.position.y, 0.0);

	camera_x = bow.objectmatrix.position.x + 0.2;
	camera_y = bow.objectmatrix.position.y + 0.2;
	camera_z = bow.objectmatrix.position.z + 0.6;

	otherCamera_x = otherBow.objectmatrix.position.x + 0.2;
	otherCamera_y = otherBow.objectmatrix.position.y + 0.2;
	otherCamera_z = otherBow.objectmatrix.position.z + 0.6;

	DataThread = CreateThread(NULL, 0, DataComm, NULL, 0, NULL);
	if (DataThread == NULL) closesocket(sock);
	else CloseHandle(DataThread);
	return true;
}