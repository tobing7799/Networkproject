#include "Common.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <time.h>

#define SERVERPORT 9000
#define BUFSIZE    512

const int CIRCLENUMWIDTH = 2;
const int CIRCLENUMHEIGHT = 2;
const int CIRCLENUM = CIRCLENUMWIDTH * CIRCLENUMHEIGHT;
const int stage1MaxSpeed = 21;
const int stage2MaxSpeed = 41;
const int stage3MaxSpeed = 91;

int stage = 0; // 스테이지 넘버

CRITICAL_SECTION cs;

enum  CIRCLE_STATE {
	CIRCLE_OFF = 0,
	CIRCLE_ON,
	CIRCLE_PARTICLE
};

struct InPacket {
	float x_angle, y_angle;
	glm::vec3 arrowPosition;
	glm::vec3 arrowRotation;
};

struct Packet {
	float x_angle, y_angle; // 플레이어의 시야 각도 값
	glm::vec3 arrowPosition; // 화살의 좌표 값;
	glm::vec3 arrowRotation; // 화살의 회전 값;
	short total_score; // 플레이어의 현재 점수 Byte,Byte
	short wind_dir; // 바람의 방향
	float wind_speed; // 바람의 세기
	short circleState[CIRCLENUM]; // 과녁의 상태
	int stage;
	short winCount=0; 
};

struct InitPacket {
	glm::vec3 circleCenter[CIRCLENUM]; // 과녁의 중앙의 위치 값
	glm::vec3 player1Pos; // 플레이어 1의 위치
	glm::vec3 player2Pos; // 플레이어 2의 위치
};

// 소켓과 함께 인덱스를 넘겨줌
struct SocketWithIndex {
	SOCKET sock;
	int index;
};

short g_circleState[CIRCLENUM];

InitPacket InitializePacket();
DWORD WINAPI windTimer(short winddir, float windspeed);
void CircleMgr(const glm::vec3& pos, int index);
void nextStage();

bool ArrowCheck(const glm::vec3& pos, int circleIndex, int index);

//std::random_device rd;
//std::default_random_engine dre(rd());

std::default_random_engine dre;
std::uniform_real_distribution<float> urd{ 50.f, 90.f };

InitPacket g_InitPacket;
glm::vec3 g_circleCenter[CIRCLENUM];

int client_index = 0;

int clientScore[2];

HANDLE hRecvEvent[2];
HANDLE hSendEvent[2];
HANDLE hThread[2];

InPacket g_InPacket[2];
Packet g_Packet[2];

time_t windTime;
HANDLE hWindThread;

short windDir;
float windSpeed;
int	client0_win_count;
int	client1_win_count;

DWORD WINAPI ClientMgr(LPVOID arg) { // arg로 SocketWithIndex가 넘어옴
	SocketWithIndex* swi = (SocketWithIndex*)arg;
	SOCKET client_sock = swi->sock;
	int index = swi->index;

	int opositeIndex;
	if (index == 1)
		opositeIndex = 0;
	else
		opositeIndex = 1;

	DWORD retval;

	// 초기화값 전송
	DWORD wh = MAKEWORD(CIRCLENUMWIDTH, CIRCLENUMHEIGHT);
	retval = send(client_sock, (char*)&wh, sizeof(DWORD), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 1;
	}
	if (index == 0) {
		g_InitPacket.player1Pos = glm::vec3(1.f, 0, 0);
		g_InitPacket.player2Pos = glm::vec3(-1.f, 0, 0);
	}
	else {
		g_InitPacket.player2Pos = glm::vec3(1.f, 0, 0);
		g_InitPacket.player1Pos = glm::vec3(-1.f, 0, 0);
	}
	retval = send(client_sock, (char*)&g_InitPacket, sizeof(InitPacket), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 1;
	}

	while (1) {
		retval = WaitForSingleObject(hRecvEvent[index], INFINITE);
		if (retval != WAIT_OBJECT_0) break;

		retval = recv(client_sock, (char*)&g_InPacket[index], sizeof(InPacket), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 충돌체크 후 점수계산
		CircleMgr(g_InPacket[index].arrowPosition, index);

		// 현재 인덱스의 클라이언트 정보를 상대 클라이언트에게 넘겨줄 패킷에 저장
		g_Packet[opositeIndex].arrowPosition = g_InPacket[index].arrowPosition;
		g_Packet[opositeIndex].arrowRotation = g_InPacket[index].arrowRotation;
		g_Packet[opositeIndex].x_angle = g_InPacket[index].x_angle;
		g_Packet[opositeIndex].y_angle = g_InPacket[index].y_angle;
		short packetScore = MAKEWORD(clientScore[index], clientScore[opositeIndex]);
		g_Packet[index].total_score = packetScore;

		if (index == 0) { // 0번 클라면 1번 클라 recv이벤트
			SetEvent(hRecvEvent[opositeIndex]);
			ResetEvent(hRecvEvent[index]);
		}
		else { // 1번 클라면 0번 클라 send이벤트
			SetEvent(hSendEvent[opositeIndex]);
			ResetEvent(hRecvEvent[index]);
		}
		retval = WaitForSingleObject(hSendEvent[index], INFINITE);
		if (retval != WAIT_OBJECT_0) break;
		// 클라이언트로 데이터 송신
		// 이벤트는 추후에 추가
		retval = send(client_sock, (char*)&g_Packet[index], sizeof(Packet), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		// send 추가해야함
		if (index == 0) { // 0번 클라면 1번 클라 recv이벤트
			SetEvent(hSendEvent[opositeIndex]);
			ResetEvent(hSendEvent[index]);
		}
		else { // 1번 클라면 0번 클라 send이벤트
			SetEvent(hRecvEvent[opositeIndex]);
			ResetEvent(hSendEvent[index]);
		}
	}

	closesocket(client_sock);
	return 0;
}

DWORD WINAPI windTimer(LPVOID arg) {
	while (1) {
		time_t currentTime = time(NULL);
		if (currentTime - windTime >= 10)
		{
			EnterCriticalSection(&cs);
			if (stage == 0)
			{
				windSpeed = 0.0;
			}
			else if (stage == 1)
			{
				windSpeed = ((float)(rand() % stage1MaxSpeed) / 10);
			}
			else if (stage == 2)
			{
				windSpeed = ((float)(rand() % stage2MaxSpeed) / 10);
			}
			else if (stage == 3)
			{
				windSpeed = ((float)(rand() % stage3MaxSpeed) / 10);
			}
			windDir = rand() % 9;
			windTime = currentTime;
			g_Packet[0].wind_dir = windDir;
			g_Packet[0].wind_speed = windSpeed;
			g_Packet[1].wind_dir = windDir;
			g_Packet[1].wind_speed = windSpeed;
			std::cout << windDir << std::endl;
			LeaveCriticalSection(&cs);
			printf("0클라 : %d 1클라 : %d\n", clientScore[0], clientScore[1]);
		}
	}
}


int main(int argc, char* argv[])
{
	int retval;

	InitializeCriticalSection(&cs);

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	g_InitPacket = InitializePacket();

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock[2];
	struct sockaddr_in clientaddr[2];
	int addrlen[2];
	SocketWithIndex swi[2];

	addrlen[0] = sizeof(clientaddr[0]);
	client_sock[0] = accept(listen_sock, (struct sockaddr*)&clientaddr[0], &addrlen[0]);
	if (client_sock[0] == INVALID_SOCKET) {
		err_quit("accept()");
	}
	swi[0].sock = client_sock[0];
	swi[0].index = 0;

	// 2번째 클라이언트도 받게 대기

	addrlen[1] = sizeof(clientaddr[1]);
	client_sock[1] = accept(listen_sock, (struct sockaddr*)&clientaddr[1], &addrlen[1]);
	if (client_sock[1] == INVALID_SOCKET) {
		err_quit("accept()");
	}
	swi[1].sock = client_sock[1];
	swi[1].index = 1;

	// 통신을 위한 ClinetMgr 시작
	hThread[0] = CreateThread(NULL, 0, ClientMgr, &swi[0], 0, NULL);
	hThread[1] = CreateThread(NULL, 0, ClientMgr, &swi[1], 0, NULL);
	
	windTime = time(NULL);
	hWindThread = CreateThread(NULL, 0, windTimer, NULL, 0, NULL);
	if (hWindThread != NULL) CloseHandle(hWindThread);

	hRecvEvent[0] = CreateEvent(NULL, TRUE, TRUE, NULL);
	hSendEvent[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
	hRecvEvent[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
	hSendEvent[1] = CreateEvent(NULL, TRUE, FALSE, NULL);

	WaitForMultipleObjects(2, hThread, false, INFINITE);

	CloseHandle(hRecvEvent[0]);
	CloseHandle(hRecvEvent[1]);
	CloseHandle(hSendEvent[0]);
	CloseHandle(hSendEvent[1]);

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

InitPacket InitializePacket()
{
	InitPacket packet;

	for (int i = 0; i < CIRCLENUMWIDTH; ++i) {
		for (int j = 0; j < CIRCLENUMHEIGHT; ++j) {
			//g_circleCenter[5 * i + j] = glm::vec3((i - CIRCLENUMWIDTH / 2) * 10.f, (j - CIRCLENUMHEIGHT / 2) * 10.f, urd(dre));
			g_circleCenter[2 * i + j] = glm::vec3((i - CIRCLENUMWIDTH / 2) * 3.f, (j) * 3.f, 40.f);
			g_circleState[2 * i + j] = CIRCLE_ON;
		}
	}
	//std::sort(g_circleCenter, g_circleCenter + CIRCLENUM, [](const glm::vec3& a, const glm::vec3& b) {
	//	return a.z > b.z;
	//	});
	for (int i = 0; i < CIRCLENUM; ++i) {
		packet.circleCenter[i] = g_circleCenter[i];
		g_Packet[0].circleState[i] = g_Packet[1].circleState[i] = g_circleState[i];
		printf("%f, %f %f\n", packet.circleCenter[i].x, packet.circleCenter[i].y, packet.circleCenter[i].z);
	}
	packet.player1Pos = glm::vec3(-1.f, 0, 0);
	packet.player2Pos = glm::vec3(1.f, 0, 0);

	return packet;
}

void nextStage()
{
	if (clientScore[0] < clientScore[1])
	{
		client1_win_count++;
	}
	else if (clientScore[0] > clientScore[1])
	{
		client0_win_count++;
	}
	else
	{
		client0_win_count++;
		client1_win_count++;
	}

	g_Packet[0].winCount = MAKEWORD(client0_win_count, client1_win_count);
	g_Packet[1].winCount = MAKEWORD(client1_win_count, client0_win_count);

	stage++;
	if (stage > 3) {
		stage = 0;
	}
	g_Packet[0].stage = stage;
	g_Packet[1].stage = stage;
	clientScore[0] = 0;
	clientScore[1] = 0;
}

void CircleMgr(const glm::vec3& pos, int index)
{
	int count{};
	for (int i = 0; i < CIRCLENUM; ++i)
	{
		if (g_circleState[i] == CIRCLE_OFF) {
			count++;
			continue;
		}
		else if (g_circleState[i] == CIRCLE_ON)
		{
			if (ArrowCheck(pos, i, index)) {
				g_Packet[0].circleState[i] = g_Packet[1].circleState[i] = g_circleState[i] = CIRCLE_PARTICLE;
				break;
			}
		}
		else if (g_circleState[i] == CIRCLE_PARTICLE && index == 0)
		{
			g_Packet[0].circleState[i] = g_Packet[1].circleState[i] = g_circleState[i] = CIRCLE_OFF;
		}
	}
	if (count == CIRCLENUM) {
		if (index == 1) {
			nextStage();
			for (int i = 0; i < CIRCLENUM; ++i) {
				g_Packet[0].circleState[i] = g_Packet[1].circleState[i] = g_circleState[i] = CIRCLE_ON;
			}
		}
	}
}

bool ArrowCheck(const glm::vec3& pos, int circleIndex, int index)
{
	int score{};
	//if (pos.z < g_circleCenter[circleIndex].z)
		//return false;

	if (pos.z >= 40.f) {
		score = 10;
		for (int i = 0; i < 10; ++i) {
			if (sqrt(pow(g_circleCenter[circleIndex].x - pos.x, 2.0) + pow(g_circleCenter[circleIndex].y - pos.y, 2.0)) <= (i * 0.1 + 0.1)) // wind_x, wind_y 일단 제외
			{
				if (score > i)
					score = i;
			}
		}
		if (score == 10)
			return false;
		clientScore[index] += 10 - score;
		//printf("%d 클라가 %d점 오름\n", index, 10 - score);
		return true;
	}
	return false;
}