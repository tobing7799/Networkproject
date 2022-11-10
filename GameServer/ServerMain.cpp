#include "Common.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#define SERVERPORT 9000
#define BUFSIZE    512

const int CIRCLENUMWIDTH = 5;
const int CIRCLENUMHEIGHT = 5;
const int CIRCLENUM = CIRCLENUMWIDTH * CIRCLENUMHEIGHT;

enum  CIRCLE_STATE {
	CIRCLE_ON = 0,
	CIRCLE_PARTICLE,
	CIRCLE_OFF
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

short g_circleState[CIRCLENUM];

InitPacket InitializePacket();

//std::random_device rd;
//std::default_random_engine dre(rd());

std::default_random_engine dre;
std::uniform_real_distribution<float> urd{ 50.f, 90.f };

InitPacket g_InitPacket;

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
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while (1) {
			// ������
		}

		// ���� �ݱ�
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
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
			//printf("%f, %f %f\n", packet.circleCenter[5 * i + j].x, packet.circleCenter[5 * i + j].y, packet.circleCenter[5 * i + j].z);
		}
	}

	packet.player1Pos = glm::vec3(-10.f, 0, 0);
	packet.player2Pos = glm::vec3(10.f, 0, 0);

	return packet;
}