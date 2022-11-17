#include "Common.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <time.h>
#include <math.h>

#define SERVERPORT 9000
#define BUFSIZE    512

const int CIRCLENUMWIDTH = 5;
const int CIRCLENUMHEIGHT = 5;
const int CIRCLENUM = CIRCLENUMWIDTH * CIRCLENUMHEIGHT;

int stage = 0; // �������� �ѹ�

enum  CIRCLE_STATE {
	CIRCLE_ON = 0,
	CIRCLE_PARTICLE,
	CIRCLE_OFF
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
	short total_score; // �÷��̾��� ���� ����
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

//std::random_device rd;
//std::default_random_engine dre(rd());

std::default_random_engine dre;
std::uniform_real_distribution<float> urd{ 50.f, 90.f };

InitPacket g_InitPacket;

int client_index = 0;

HANDLE hReadEvent[2];
HANDLE hWriteEvent[2];

InPacket g_InPacket[2];
Packet g_Packet[2];

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
	// �ʱ�ȭ���� ��� �����°� �������
	retval = send(client_sock, (char*)&g_InitPacket, sizeof(InitPacket), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 1;
	}

	while (1) {
		// Ŭ���̾�Ʈ�κ��� ������ ����
		// �̺�Ʈ�� ���Ŀ� �߰�
		retval = recv(client_sock, (char*)&g_InPacket[index], sizeof(InPacket), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// ���� �ε����� Ŭ���̾�Ʈ ������ ��� Ŭ���̾�Ʈ���� �Ѱ��� ��Ŷ�� ����
		g_Packet[opositeIndex].arrowPosition = g_InPacket[index].arrowPosition;
		g_Packet[opositeIndex].arrowRotation = g_InPacket[index].arrowRotation;
		g_Packet[opositeIndex].x_angle = g_InPacket[index].x_angle;
		g_Packet[opositeIndex].y_angle=	g_InPacket[index].y_angle;

		// Ŭ���̾�Ʈ�� ������ �۽�
		// �̺�Ʈ�� ���Ŀ� �߰�
		retval = send(client_sock, (char*)&g_Packet[index], sizeof(Packet), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	closesocket(client_sock);
	return 0;

	//if (client_index < 2) {
	//	// �ƹ������ų� �Ѹ��϶� ó���ϴ� �κ� �־�ߵ�.
	//}
	//while (client_index == 2) { // �θ��϶�
	//	for (int i = 0; i < 2; ++i) {
	//		retval = WaitForSingleObject(hReadEvent[i], INFINITE);
	//		if (retval != WAIT_OBJECT_0)
	//			break;
	//	}
	//}

	//return 0;
}



void windTimer(short winddir, float windspeed) {
	static int wind_timer = 1000;
	if (wind_timer <= 0)
	{
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
		wind_timer = 1000;
		winddir = rand() % 9;
		std::cout << winddir << std::endl;
	}
	else
	{
		wind_timer -= 1;
	}
}


int main(int argc, char* argv[])
{
	int retval;

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
	HANDLE hThread;

	while (1) {
		// accept()
		addrlen[0] = sizeof(clientaddr[0]);
		client_sock[0] = accept(listen_sock, (struct sockaddr*)&clientaddr[0], &addrlen[0]);
		if (client_sock[0] == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		swi[0].sock = client_sock[0];
		swi[0].index = 0;

		// 2��° Ŭ���̾�Ʈ�� �ް� ���

		addrlen[1] = sizeof(clientaddr[1]);
		client_sock[1] = accept(listen_sock, (struct sockaddr*)&clientaddr[1], &addrlen[1]);
		if (client_sock[1] == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		swi[1].sock = client_sock[1];
		swi[1].index = 0;

		// ����� ���� ClinetMgr ����
		hThread = CreateThread(NULL, 0, ClientMgr, &swi[0], 0, NULL);
		if (hThread == NULL) closesocket(client_sock[0]);
		else CloseHandle(hThread);
		hThread = CreateThread(NULL, 0, ClientMgr, &swi[1], 0, NULL);
		if (hThread == NULL) closesocket(client_sock[1]);
		else CloseHandle(hThread);

		// ������ Ŭ���̾�Ʈ ���� ���
		//char addr[INET_ADDRSTRLEN];
		//inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		//printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", addr, ntohs(clientaddr.sin_port));

		// ���� �ݱ�
		closesocket(client_sock[0]);
		closesocket(client_sock[1]);
		//printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n", addr, ntohs(clientaddr.sin_port));
	}

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
			packet.circleCenter[5 * i + j] = glm::vec3((i - CIRCLENUMWIDTH / 2) * 10.f, (j - CIRCLENUMHEIGHT / 2) * 10.f,  urd(dre));
			g_circleState[5 * i + j] = CIRCLE_ON;
			printf("%f, %f %f\n", packet.circleCenter[5 * i + j].x, packet.circleCenter[5 * i + j].y, packet.circleCenter[5 * i + j].z);
		}
	}

	packet.player1Pos = glm::vec3(-10.f, 0, 0);
	packet.player2Pos = glm::vec3(10.f, 0, 0);

	return packet;
}