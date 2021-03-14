#include "TCPClient.h"
#include <chrono>

TCPClient::TCPClient(string ip, string PORT) {

	this->connectedStatus = true;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
		printf("WSAStartup failed with error: %d\n", iResult);

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(ip.c_str(), PORT.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		this->connectedStatus = true;
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
		this->connectedStatus = false;
	}

	// Ping Check (RTT)
	if (this->connectedStatus) {
		this->calculateRTT();
	}
}

void TCPClient::update(glm::vec3 position, float yaw, Renderer* renderer) {
	// Send Player info
	sendbuf = to_string(position.x) + " " + to_string(position.y) + " "
		+ to_string(position.z) + " " + to_string(yaw);

	iResult = send(ConnectSocket, sendbuf.data(), sendbuf.size(), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	
	string playerId;
	glm::vec4 output = readBufToVectors(recvbuf, playerId);

	// update player
	if (renderer->players.count(playerId)) { 
		Entity* player = renderer->players.at(playerId);
		player->modelMatrix = glm::translate(glm::mat4(1), glm::vec3(output.x, output.y - 2.5f, output.z));
		player->modelMatrix = glm::rotate(player->modelMatrix, glm::radians(output.w), glm::vec3(0, 1, 0));
	}
	// create new player entity from cache in loader
	else { 
		cout << "New Player joined server: " << playerId << endl;
		
		glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(output.x, output.y, output.z));
		tempModelMatrix = glm::rotate(tempModelMatrix, glm::radians(output.w), glm::vec3(0, 1, 0));

		Entity* newPlayer = readOBJ_better("gameFiles/Player.obj", "gameFiles/Character.jpg", nullptr, tempModelMatrix);
		renderer->players.insert(pair<string, Entity*>(playerId, newPlayer));

		cout << "New Player spawned at: " << glm::to_string(output) << endl;
	}
}

glm::vec4 TCPClient::readBufToVectors(const char* buffer, string& playerId) {
	//first 5 values, (id, x, y, z, yaw) for player
	char* pEnd;
	string input = string(buffer);
	playerId = input.substr(0, input.find(" "));
	float x = strtof(buffer + playerId.length(), &pEnd);
	float y = strtof(pEnd, &pEnd);
	float z = strtof(pEnd, &pEnd);
	float yaw = strtof(pEnd, &pEnd);
	return glm::vec4(x, y, z, yaw);
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

void TCPClient::calculateRTT() {
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;

	milliseconds ms = duration_cast<milliseconds>(chrono::system_clock::now().time_since_epoch());
	sendbuf = "RTT_CHECK";

	iResult = send(ConnectSocket, sendbuf.data(), sendbuf.size(), 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
	}

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	string input = string(recvbuf);
	if (input == "RTT_CHECK") {
		milliseconds now = duration_cast<milliseconds>(chrono::system_clock::now().time_since_epoch());
		long long RTT = now.count() - ms.count();
		cout << "RTT: " << RTT << "ms" << endl;
	}
}