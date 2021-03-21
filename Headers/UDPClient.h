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
#include <iostream>
#include <chrono>
using namespace std;

extern string playerName;

#define DEFAULT_BUFLEN 512

struct playerInfo {
	glm::vec4 modelMatrix;
	string playerId;
};

class UDPClient
{
public:
	UDPClient(string server, string PORT) {
		this->connectedStatus = true;
		if (server == "" || PORT == "") {
			this->connectedStatus = false;
			return;
		}

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != NO_ERROR) {
			wprintf(L"WSAStartup failed with error: %d\n", iResult);
			bool connectedStatus = false;
			return;
		}

		// Create a socket for sending data
		ConnectSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (ConnectSocket == INVALID_SOCKET) {
			wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
			bool connectedStatus = false;
			WSACleanup();
		}

		// Set up the dest structure with the IP address of
		// the receiver and the specified port number.
		dest.sin_family = AF_INET;
		dest.sin_port = htons(9998);
		inet_pton(AF_INET, server.c_str(), &dest.sin_addr.s_addr); // (might not work with hostnames?)


		// Ping Check (RTT)
		if (this->connectedStatus) {
			this->calculateRTT();
		}

		sendbuf = "CONNECT&0.0&0.0&0.0&0.0&" + playerName;
		cout << "sending: " << sendbuf << endl;
		iResult = sendto(ConnectSocket, sendbuf.data(), static_cast<int>(sendbuf.size()), 0, (SOCKADDR*)&dest, sizeof(dest));
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
		}
	}

	playerInfo update(glm::vec3 position, float yaw)
	{
		// Send Player info
		sendbuf = "UPDATE&" + to_string(position.x) + "&" + to_string(position.y) + "&"
			+ to_string(position.z) + "&" + to_string(yaw);

		iResult = sendto(ConnectSocket, sendbuf.data(), static_cast<int>(sendbuf.size()), 0, (SOCKADDR*)&dest, sizeof(dest));
		if (iResult == SOCKET_ERROR) {
			printf("send failed with error: %d\n", WSAGetLastError());
		}

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		//iResult = recvfrom(ConnectSocket, recvbuf, recvbuflen, 0, reinterpret_cast<SOCKADDR*>(&dest), (int *)sizeof(dest));

		playerInfo data = serializeBuffer(recvbuf);
		
		return data;
	}

	playerInfo serializeBuffer(const char* buffer) {
		//first 5 values, (id, x, y, z, yaw) for player
		string delimiter = "&";
		string input = string(buffer);
		string playerId = input.substr(0, input.find(delimiter));
		input.erase(0, input.find(delimiter) + delimiter.length());

		float x = stof(input.substr(0, input.find("&")));
		input.erase(0, input.find(delimiter) + delimiter.length());

		float y = stof(input.substr(0, input.find("&")));
		input.erase(0, input.find(delimiter) + delimiter.length());

		float z = stof(input.substr(0, input.find("&")));
		input.erase(0, input.find(delimiter) + delimiter.length());

		float yaw = stof(input.substr(0, input.find("&")));

		playerInfo data{
			glm::vec4(x, y, z, yaw),
			playerId
		};

		return data;
	}

	void cleanUp()
	{
		// When the application is finished sending, close the socket.
		iResult = closesocket(ConnectSocket);
		if (iResult == SOCKET_ERROR) {
			wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
			WSACleanup();
			return;
		}
	}

	void calculateRTT() {
		using std::chrono::duration_cast;
		using std::chrono::milliseconds;

		milliseconds ms = duration_cast<milliseconds>(chrono::system_clock::now().time_since_epoch());
		sendbuf = "RTT_CHECK";

		//iResult = send(ConnectSocket, sendbuf.data(), sendbuf.size(), 0);
		iResult = sendto(ConnectSocket, sendbuf.data(), static_cast<int>(sendbuf.size()), 0, (SOCKADDR*)&dest, sizeof(dest));
		if (iResult == SOCKET_ERROR) {
			printf("Failed RTT datagram send: %d\n", WSAGetLastError());
		}

		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		string input = string(recvbuf, iResult);
		cout << input;
		//if (input == "RTT_CHECK") {
		milliseconds now = duration_cast<milliseconds>(chrono::system_clock::now().time_since_epoch());
		long long RTT = now.count() - ms.count();
		cout << " RTT: " << RTT << "ms" << endl;
		//}
	}

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;

	sockaddr_in dest;

	string sendbuf;
	char recvbuf[DEFAULT_BUFLEN];
	int iResult = NULL;
	int recvbuflen = DEFAULT_BUFLEN;

	bool connectedStatus;

};