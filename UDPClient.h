#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include <glm/gtx/string_cast.hpp>
#include <string>
#include <chrono>
#include "Renderer.h"


#define DEFAULT_BUFLEN 512


class UDPClient
{
public:
	UDPClient(string server, string PORT);

	void update(glm::vec3 position, float yaw, Renderer* renderer);

	void cleanUp();

	void calculateRTT();

	glm::vec4 serializeBuffer(const char* buffer, string& playerId);

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;

	sockaddr_in dest;

	string sendbuf;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult = NULL;
	int recvbuflen = DEFAULT_BUFLEN;

	bool connectedStatus;
};