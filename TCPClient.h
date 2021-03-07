#pragma once
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <string>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512


#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "Renderer.h"

class TCPClient {
public:
	TCPClient(const char* ip, const char* PORT);

	void update(glm::vec3 position, float yaw, Renderer* renderer);

	glm::vec4 readBufToVectors(const char* buffer, string& playerId);

	void cleanUp();

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	std::string sendbuf;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	bool connectedStatus;
};

