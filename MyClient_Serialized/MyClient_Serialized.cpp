/*
Yousef Majidi
101306207
BTN415
March 18, 2022
*/


#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <random>
#include <thread>
#include <vector>
#include "Player.h"


using std::cout;
using std::cin;
using std::endl;
using std::string;

const string CLIENT_NAME = "Yousef";
std::vector<Player*>* players{};

float randNum(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distr(min, max);
	return distr(gen);
}

void main()
{
	// starts Winsock DLLs
	WSADATA wsaData;
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
	{
		return;
	}

	// initializes socket. SOCK_STREAM: TCP
	SOCKET ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}

	// Connect socket to specified server
	sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET;						// Address family type itnernet
	SvrAddr.sin_port = htons(27000);					// port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	// IP address
	if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR)
	{
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}

	// receives Rxbuffer
	int count = 0;
	Player player(Location(), CLIENT_NAME);
	char* sendBuffer{};
	char* recvBuffer{};
	float x{}, y{}, z{};

	while (count < 25)
	{
		x = randNum(0, 10);
		y = randNum(0, 10);
		z = randNum(0, 10);
		player.updateLocation(Location(x,y,z));
		sendBuffer = reinterpret_cast<char*>(&player);
		// player.serialize(TxBuffer);
		// std::memcpy(TxBuffer, &player, sizeof(player));
		send(ClientSocket, sendBuffer, sizeof(Player), 0);
		recvBuffer = (char*)std::malloc(1024);
		recv(ClientSocket, recvBuffer, 1024, 0);
		players = reinterpret_cast<std::vector<Player*>*>(recvBuffer);
		if (!players->empty())
		{
			players->back()->print();
			/*std::for_each(players->cbegin(), players->cend(), [](const Player* player)
				{
					if (player->getName() != CLIENT_NAME)
					{
						player->print();
					}
				});*/
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	cout << "End of transmission. Terminating..." << endl;
	// closes connection and socket
	closesocket(ClientSocket);
	// frees Winsock DLL resources
	WSACleanup();
}