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
	float x_angle, y_angle; // 플레이어의 시야 각도 값
	glm::vec3 arrowPosition; // 화살의 좌표 값;
	glm::vec3 arrowRotation; // 화살의 회전 값;
	short total_score; // 플레이어의 현재 점수
	short wind_dir; // 바람의 방향
	float wind_speed; // 바람의 세기
	short circleState[CIRCLENUM]; // 과녁의 상태
};

struct InitPacket {
	glm::vec3 circleCenter[CIRCLENUM]; // 과녁의 중앙의 위치 값
	glm::vec3 player1Pos; // 플레이어 1의 위치
	glm::vec3 player2Pos; // 플레이어 2의 위치
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

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1) {
			// 데이터
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

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
			packet.circleCenter[5 * i + j] = glm::vec3((i - CIRCLENUMWIDTH / 2) * 10.f, (j - CIRCLENUMHEIGHT / 2) * 10.f,  urd(dre));
			g_circleState[5 * i + j] = CIRCLE_ON;
			//printf("%f, %f %f\n", packet.circleCenter[5 * i + j].x, packet.circleCenter[5 * i + j].y, packet.circleCenter[5 * i + j].z);
		}
	}

	packet.player1Pos = glm::vec3(-10.f, 0, 0);
	packet.player2Pos = glm::vec3(10.f, 0, 0);

	return packet;
}