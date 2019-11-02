#include "TCPClient.h"
#include <stdlib.h>
#include <iostream>

TCPClient::TCPClient(const char* ip, const char* PORT) {

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
		printf("WSAStartup failed with error: %d\n", iResult);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(ip, PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
	}

	//connected, can now send data
}

glm::vec4 TCPClient::update(glm::vec3 position, float yaw) {
	// Send an initial buffer
	sendbuf = std::to_string(position.x) + " "
		+ std::to_string(position.y) + " "
		+ std::to_string(position.z) + " "
		+ std::to_string(yaw) + ",";

	iResult = send(ConnectSocket, sendbuf.data(), sendbuf.size(), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}

	printf("Bytes Sent: %ld , sent:%s\n", iResult, sendbuf.c_str());

	// This will block thread if nothing is recieved! to be fixed
	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	/*do {

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);*/

	printf("Recieved: %s \n", recvbuf);
	glm::vec4 output = readBufToVectors(recvbuf);
	return output;
}

void TCPClient::cleanUp() {
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}
	closesocket(ConnectSocket);
	WSACleanup();
}

glm::vec4 TCPClient::readBufToVectors(const char * buffer) {
	//for now read only first 4 values (one vector) Data is CSV
	char* pEnd;
	float x = strtof(buffer, &pEnd);
	float y = strtof(pEnd, &pEnd);
	float z = strtof(pEnd, &pEnd);
	float yaw = strtof(pEnd, &pEnd);
	return glm::vec4(x, y, z, yaw);
}