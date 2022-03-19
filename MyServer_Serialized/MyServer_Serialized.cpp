/*
Yousef Majidi
101306207
BTN415
March 11, 2022
*/

#pragma comment(lib, "Ws2_32.lib")
#include <windows.networking.sockets.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <mutex>
#include "../MyClient_Serialized/Player.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

const int MAX_SOCKETS = 2;
SOCKET Aux_Socket;
SOCKET ClientSockets[MAX_SOCKETS + 1] = { SOCKET_ERROR };
bool Active_Sockets[MAX_SOCKETS + 1] = { false };
vector <Player> players{};
vector <std::thread> clients{};
std::mutex myMutex{};


int find_available_socket(void)
{
	int socket_number = MAX_SOCKETS;
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (!Active_Sockets[i])
		{
			socket_number = i;
			break;
		}
	}
	return socket_number;
}

//void deserialize(char* data)
//{
//	std::lock_guard<std::mutex> guard(myMutex);
//
//	float x{}, y{}, z{};
//	string name{};
//
//	float* q = reinterpret_cast<float*>(data);
//	x = *q; q++;
//	y = *q; q++;
//	z = *q; q++;
//
//	char* p = reinterpret_cast<char*>(q);
//	int i = 0;
//	while (*p > 0)
//	{
//		name += *p;
//		p++;
//		i++;
//	}
//
//	auto found = std::find_if(players.begin(), players.end(), [name](Player* player) {
//		return name == player->getName();
//		});
//	if (found == players.end())
//	{
//		players.push_back(new Player(x, y, z, name));
//		return;
//	}
//
//	(*found)->updateLocation(x, y, z);
//}
void handle_client(int idx)
{
	//const std::lock_guard<std::mutex> guard(myMutex);
	Active_Sockets[idx] = true;
	int count = 0;
	char recvBuffer[128] = {};
	char* sendBuffer{};
	while (true)
	{
		if (recv(ClientSockets[idx], recvBuffer, sizeof(Player), 0) == 0)
			return;
		//deserialize(RxBuffer);
		Player* newPlayer = reinterpret_cast<Player*>(recvBuffer);
		const auto name = newPlayer->getName();
		auto found = std::find_if(players.begin(), players.end(), [&name](const Player& player)
			{
				return name == player.getName();
			});
		const std::lock_guard<std::mutex> guard(myMutex);
		if (found == players.cend())
		{
			players.push_back(*newPlayer);
			cout << std::left << std::setw(3) << name << " added to the collection of players" << endl;
		}
		else
		{
			(*found).updateLocation(newPlayer->getLocation());
			cout << std::left << std::setw(3) << count << " - ";
			(*found).print();
		}
		sendBuffer = reinterpret_cast<char*>(&players);
		send(ClientSockets[idx], sendBuffer, sizeof(players), 0);
		++count;
	}
	cout << "communication finished..." << endl;
	closesocket(ClientSockets[idx]);
	Active_Sockets[idx] = false;
}
int main()
{
	int Socket_Number;
	struct sockaddr_in SvrAddr {};				//structure to hold servers IP address
	SOCKET ListenSocket;						//socket descriptors
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return -1;

	//create welcoming socket at port and bind local address
	if ((ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		return -1;


	SvrAddr.sin_family = AF_INET;			//set family to internet
	SvrAddr.sin_addr.s_addr = INADDR_ANY;   //inet_addr("127.0.0.1");	//set the local IP address
	SvrAddr.sin_port = htons(27000);							//set the port 

	if ((bind(ListenSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR)
	{
		closesocket(ListenSocket);
		WSACleanup();
		return -1;
	}

	//Specify the maximum number of clients that can be queued
	if (listen(ListenSocket, 1) == SOCKET_ERROR)
	{
		closesocket(ListenSocket);
		return -1;
	}

	//Main server loop - accept and handle requests from clients
	std::cout << "Ready to accept a connection" << std::endl;
	
	

	for (int i = 0; i < MAX_SOCKETS; i++)
	{

		//wait for an incoming connection from a client
		if ((Aux_Socket = accept(ListenSocket, NULL, NULL)) == SOCKET_ERROR)
		{
			//Provide a message on the console to the user when the connection is rejected
			cout << "connection rejected!, IP address unregistered" << endl;
			return -1;
		}
		else
		{
			Socket_Number = find_available_socket();
			if (Socket_Number < MAX_SOCKETS)
			{
				std::cout << "Client Connection Made\n" << std::endl;
				ClientSockets[Socket_Number] = Aux_Socket;
				clients.push_back(std::thread(handle_client, Socket_Number));
			}
		}
	}

	for (auto i = 0u; i < MAX_SOCKETS; i++)
		clients[i].join();

	closesocket(ListenSocket);
	WSACleanup();
	return 0;
}