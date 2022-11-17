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

int stage = 0; // 스테이지 넘버

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

// 소켓과 함께 인덱스를 넘겨줌
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
	// 초기화값을 어디서 보내는게 좋을까용
	retval = send(client_sock, (char*)&g_InitPacket, sizeof(InitPacket), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 1;
	}

	while (1) {
		// 클라이언트로부터 데이터 수신
		// 이벤트는 추후에 추가
		retval = recv(client_sock, (char*)&g_InPacket[index], sizeof(InPacket), MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 현재 인덱스의 클라이언트 정보를 상대 클라이언트에게 넘겨줄 패킷에 저장
		g_Packet[opositeIndex].arrowPosition = g_InPacket[index].arrowPosition;
		g_Packet[opositeIndex].arrowRotation = g_InPacket[index].arrowRotation;
		g_Packet[opositeIndex].x_angle = g_InPacket[index].x_angle;
		g_Packet[opositeIndex].y_angle=	g_InPacket[index].y_angle;

		// 클라이언트로 데이터 송신
		// 이벤트는 추후에 추가
		retval = send(client_sock, (char*)&g_Packet[index], sizeof(Packet), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
	}

	closesocket(client_sock);
	return 0;

	//if (client_index < 2) {
	//	// 아무도없거나 한명일때 처리하는 부분 넣어야됨.
	//}
	//while (client_index == 2) { // 두명일때
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

		// 2번째 클라이언트도 받게 대기

		addrlen[1] = sizeof(clientaddr[1]);
		client_sock[1] = accept(listen_sock, (struct sockaddr*)&clientaddr[1], &addrlen[1]);
		if (client_sock[1] == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		swi[1].sock = client_sock[1];
		swi[1].index = 0;

		// 통신을 위한 ClinetMgr 시작
		hThread = CreateThread(NULL, 0, ClientMgr, &swi[0], 0, NULL);
		if (hThread == NULL) closesocket(client_sock[0]);
		else CloseHandle(hThread);
		hThread = CreateThread(NULL, 0, ClientMgr, &swi[1], 0, NULL);
		if (hThread == NULL) closesocket(client_sock[1]);
		else CloseHandle(hThread);

		// 접속한 클라이언트 정보 출력
		//char addr[INET_ADDRSTRLEN];
		//inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		//printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));

		// 소켓 닫기
		closesocket(client_sock[0]);
		closesocket(client_sock[1]);
		//printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", addr, ntohs(clientaddr.sin_port));
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
			printf("%f, %f %f\n", packet.circleCenter[5 * i + j].x, packet.circleCenter[5 * i + j].y, packet.circleCenter[5 * i + j].z);
		}
	}

	packet.player1Pos = glm::vec3(-10.f, 0, 0);
	packet.player2Pos = glm::vec3(10.f, 0, 0);

	return packet;
}