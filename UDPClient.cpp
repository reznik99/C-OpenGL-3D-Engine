
#include "UDPClient.h"



UDPClient::UDPClient(string server, string PORT) {
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

	sendbuf = "CONNECT&0.0&0.0&0.0&0.0&Frank";
	iResult = sendto(ConnectSocket, sendbuf.data(), sendbuf.size(), 0, (SOCKADDR*)&dest, sizeof(dest));
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
	}
}

void UDPClient::update(glm::vec3 position, float yaw, Renderer* renderer)
{
	// Send Player info
	sendbuf = "UPDATE&" + to_string(position.x) + "&" + to_string(position.y) + "&"
		+ to_string(position.z) + "&" + to_string(yaw);

	iResult = sendto(ConnectSocket, sendbuf.data(), sendbuf.size(), 0, (SOCKADDR*)&dest, sizeof(dest));
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
	}

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	//iResult = recvfrom(ConnectSocket, recvbuf, recvbuflen, 0, reinterpret_cast<SOCKADDR*>(&dest), (int *)sizeof(dest));

	string playerId;
	glm::vec4 output = serializeBuffer(recvbuf, playerId);

	// update player
	if (renderer->players.count(playerId)) {
		Entity* player = renderer->players.at(playerId);
		player->modelMatrix = glm::translate(glm::mat4(1), glm::vec3(output.x, output.y - 8.0f, output.z));
		player->modelMatrix = glm::rotate(player->modelMatrix, glm::radians(-output.w + 90), glm::vec3(0, 1, 0));
	}
	// create new player entity from cache in loader
	else {
		cout << "New Player joined server: " << playerId << endl;

		glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(output.x, output.y, output.z));
		tempModelMatrix = glm::rotate(tempModelMatrix, glm::radians(-output.w), glm::vec3(0, 1, 0));

		Entity* newPlayer = readOBJ_better("gameFiles/Stone.obj", "gameFiles/Stone.png", nullptr, tempModelMatrix);
		renderer->players.insert(pair<string, Entity*>(playerId, newPlayer));

		cout << "New Player spawned at: " << glm::to_string(output) << endl;
	}
}

glm::vec4 UDPClient::serializeBuffer(const char* buffer, string& playerId) {
	//first 5 values, (id, x, y, z, yaw) for player
	string delimiter = "&";
	string input = string(buffer);
	playerId = input.substr(0, input.find(delimiter));
	input.erase(0, input.find(delimiter) + delimiter.length());

	float x = stof(input.substr(0, input.find("&")));
	input.erase(0, input.find(delimiter) + delimiter.length());

	float y = stof(input.substr(0, input.find("&")));
	input.erase(0, input.find(delimiter) + delimiter.length());

	float z = stof(input.substr(0, input.find("&")));
	input.erase(0, input.find(delimiter) + delimiter.length());

	float yaw = stof(input.substr(0, input.find("&")));

	return glm::vec4(x, y, z, yaw);
}

void UDPClient::cleanUp()
{
	//---------------------------------------------
	// When the application is finished sending, close the socket.
	iResult = closesocket(ConnectSocket);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}
}

void UDPClient::calculateRTT() {
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;

	milliseconds ms = duration_cast<milliseconds>(chrono::system_clock::now().time_since_epoch());
	sendbuf = "RTT_CHECK";

	//iResult = send(ConnectSocket, sendbuf.data(), sendbuf.size(), 0);
	iResult = sendto(ConnectSocket, sendbuf.data(), sendbuf.size(), 0, (SOCKADDR*)&dest, sizeof(dest));
	if (iResult == SOCKET_ERROR) {
		printf("Failed RTT datagram send: %d\n", WSAGetLastError());
	}

	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	string input = string(recvbuf, iResult);
	cout << input ;
	//if (input == "RTT_CHECK") {
		milliseconds now = duration_cast<milliseconds>(chrono::system_clock::now().time_since_epoch());
		long long RTT = now.count() - ms.count();
		cout << " RTT: " << RTT << "ms" << endl;
	//}
}