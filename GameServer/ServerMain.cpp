#include "Common.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <time.h>

#define SERVERPORT 9000
#define BUFSIZE    512

const int CIRCLENUMWIDTH = 5;
const int CIRCLENUMHEIGHT = 5;
const int CIRCLENUM = CIRCLENUMWIDTH * CIRCLENUMHEIGHT;

int stage = 0; // �������� �ѹ�

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
	float x_angle, y_angle; // �÷��̾��� �þ� ���� ��
	glm::vec3 arrowPosition; // ȭ���� ��ǥ ��;
	glm::vec3 arrowRotation; // ȭ���� ȸ�� ��;
	short total_score; // �÷��̾��� ���� ���� Byte,Byte
	short wind_dir; // �ٶ��� ����
	float wind_speed; // �ٶ��� ����
	short circleState[CIRCLENUM]; // ������ ����
};

struct InitPacket {
	glm::vec3 circleCenter[CIRCLENUM]; // ������ �߾��� ��ġ ��
	glm::vec3 player1Pos; // �÷��̾� 1�� ��ġ
	glm::vec3 player2Pos; // �÷��̾� 2�� ��ġ
};

// ���ϰ� �Բ� �ε����� �Ѱ���
struct SocketWithIndex {
	SOCKET sock;
	int index;
};

short g_circleState[CIRCLENUM];

InitPacket InitializePacket();
DWORD WINAPI windTimer(short winddir, float windspeed);
void CircleMgr(const glm::vec3& pos, int index);

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

short winddir;
float windspeed;

DWORD WINAPI ClientMgr(LPVOID arg) { // arg�� SocketWithIndex�� �Ѿ��
	SocketWithIndex* swi = (SocketWithIndex*)arg;
	SOCKET client_sock = swi->sock;
	int index = swi->index;

	int opositeIndex;
	if (index == 1)
		opositeIndex = 0;
	else
		opositeIndex = 1;

	DWORD retval;

	// �ʱ�ȭ�� ����
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

		// �浹üũ �� �������
		CircleMgr(g_InPacket[index].arrowPosition, index);

		// ���� �ε����� Ŭ���̾�Ʈ ������ ��� Ŭ���̾�Ʈ���� �Ѱ��� ��Ŷ�� ����
		g_Packet[opositeIndex].arrowPosition = g_InPacket[index].arrowPosition;
		g_Packet[opositeIndex].arrowRotation = g_InPacket[index].arrowRotation;
		g_Packet[opositeIndex].x_angle = g_InPacket[index].x_angle;
		g_Packet[opositeIndex].y_angle = g_InPacket[index].y_angle;
		short packetScore = MAKEWORD(clientScore[index], clientScore[opositeIndex]);
		g_Packet[index].total_score = packetScore;

		if (index == 0) { // 0�� Ŭ��� 1�� Ŭ�� recv�̺�Ʈ
			SetEvent(hRecvEvent[opositeIndex]);
			ResetEvent(hRecvEvent[index]);
		}
		else { // 1�� Ŭ��� 0�� Ŭ�� send�̺�Ʈ
			SetEvent(hSendEvent[opositeIndex]);
			ResetEvent(hRecvEvent[index]);
		}
		retval = WaitForSingleObject(hSendEvent[index], INFINITE);
		if (retval != WAIT_OBJECT_0) break;
		// Ŭ���̾�Ʈ�� ������ �۽�
		// �̺�Ʈ�� ���Ŀ� �߰�
		retval = send(client_sock, (char*)&g_Packet[index], sizeof(Packet), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		// send �߰��ؾ���
		if (index == 0) { // 0�� Ŭ��� 1�� Ŭ�� recv�̺�Ʈ
			SetEvent(hSendEvent[opositeIndex]);
			ResetEvent(hSendEvent[index]);
		}
		else { // 1�� Ŭ��� 0�� Ŭ�� send�̺�Ʈ
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
				windspeed = 0.0;
			}
			else if (stage == 1)
			{
				windspeed = ((float)(rand() % 21) / 10);
			}
			else if (stage == 2)
			{
				windspeed = ((float)(rand() % 41) / 10);
			}
			else if (stage == 3)
			{
				windspeed = ((float)(rand() % 91) / 10);
			}
			winddir = rand() % 9;
			windTime = currentTime;
			g_Packet[0].wind_dir = winddir;
			g_Packet[0].wind_speed = windspeed;
			g_Packet[1].wind_dir = winddir;
			g_Packet[1].wind_speed = windspeed;
			std::cout << winddir << std::endl;
			LeaveCriticalSection(&cs);
		}
	}
}


int main(int argc, char* argv[])
{
	int retval;

	InitializeCriticalSection(&cs);

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	g_InitPacket = InitializePacket();

	// ���� ����
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

	// ������ ��ſ� ����� ����
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

	// 2��° Ŭ���̾�Ʈ�� �ް� ���

	addrlen[1] = sizeof(clientaddr[1]);
	client_sock[1] = accept(listen_sock, (struct sockaddr*)&clientaddr[1], &addrlen[1]);
	if (client_sock[1] == INVALID_SOCKET) {
		err_quit("accept()");
	}
	swi[1].sock = client_sock[1];
	swi[1].index = 1;

	// ����� ���� ClinetMgr ����
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

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}

InitPacket InitializePacket()
{
	InitPacket packet;

	for (int i = 0; i < CIRCLENUMWIDTH; ++i) {
		for (int j = 0; j < CIRCLENUMHEIGHT; ++j) {
			g_circleCenter[5 * i + j] = glm::vec3((i - CIRCLENUMWIDTH / 2) * 10.f, (j - CIRCLENUMHEIGHT / 2) * 10.f, urd(dre));
			g_circleState[5 * i + j] = CIRCLE_ON;
		}
	}
	std::sort(g_circleCenter, g_circleCenter + CIRCLENUM, [](const glm::vec3& a, const glm::vec3& b) {
		return a.z > b.z;
		});
	for (int i = 0; i < CIRCLENUM; ++i) {
		packet.circleCenter[i] = g_circleCenter[i];
		printf("%f, %f %f\n", packet.circleCenter[i].x, packet.circleCenter[i].y, packet.circleCenter[i].z);
	}
	packet.player1Pos = glm::vec3(-1.f, 0, 0);
	packet.player2Pos = glm::vec3(1.f, 0, 0);

	return packet;
}

void CircleMgr(const glm::vec3& pos, int index)
{
	for (int i = 0; i < CIRCLENUM; ++i)
	{
		if (g_circleState[i] == CIRCLE_ON)
		{
			if (ArrowCheck(pos, i, index)) {
				g_Packet[0].circleState[i] = g_Packet[0].circleState[i] = g_circleState[i] = CIRCLE_PARTICLE;
			}
		}
		else if (g_circleState[i] == CIRCLE_PARTICLE && index == 0)
		{
			g_Packet[0].circleState[i] = g_Packet[1].circleState[i] = g_circleState[i] = CIRCLE_OFF;
		}
	}
}

bool ArrowCheck(const glm::vec3& pos, int circleIndex, int index)
{
	int score{};
	if (pos.z < g_circleCenter[circleIndex].z)
		return false;

	if (pos.z > g_circleCenter[circleIndex].z && pos.z < g_circleCenter[circleIndex].z + 0.1) { // �ϴ� ���� 0.1
		score = 10;
		for (int i = 0; i < 10; ++i) {
			if (sqrt(pow(g_circleCenter[circleIndex].x - pos.x, 2.0) + pow(g_circleCenter[circleIndex].y - pos.y, 2.0)) <= (i * 0.1 + 0.1)) // wind_x, wind_y �ϴ� ����
			{
				if (score > i)
					score = i;
			}
		}
		clientScore[index] += 10 - score;
		return true;
	}
	return false;
}