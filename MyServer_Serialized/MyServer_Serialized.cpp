/*
Yousef Majidi
101306207
BTN415
March 11, 2022
*/

#pragma comment(lib, "Ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <windows.networking.sockets.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <mutex>
#include "Player.h"
#include "SerializedPlayer.h"

using namespace sdds;

using std::cout;
using std::endl;
using std::string;
using std::vector;

const int MAX_SOCKETS = 2;
SOCKET Aux_Socket;
SOCKET ClientSockets[MAX_SOCKETS + 1] = { SOCKET_ERROR };
bool Active_Sockets[MAX_SOCKETS + 1] = { false };
vector <Player*> players{};
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

void handle_client(int idx)
{
	Active_Sockets[idx] = true;
	int count = 0;
	char recv_buffer[sizeof(Player)] = {};
	std::unique_ptr<char*> send_buffer(new char* {});
	
	std::unique_ptr<Player*> new_player(std::make_unique<Player*>(new Player));
	while (recv(ClientSockets[idx], recv_buffer, sizeof(Player), 0) != 0)
	{
		Player player = SerializedPlayer::player_deserializer(recv_buffer);
		auto& const name = player.name;
		auto found = std::find_if(players.begin(), players.end(), [&name](const Player* player)
			{
				return name == player->name;
			});
		const std::lock_guard<std::mutex> guard(myMutex);
		if (found == players.cend()) // player doesn't exist in the players vector
		{
			std::memcpy(*new_player, &player, sizeof(Player));
			players.push_back(*new_player);
			cout << std::left << std::setw(10) << name << " added to the collection of players" << endl;
		}
		else // player is found in the vector.. update the location
		{
			(*found)->location.update_loc(player.location.x, player.location.y, player.location.z);
			cout << std::left << std::setw(3) << count << " - ";
			printPlayerInfo(*found);
		}
		
		auto data_size = sizeof(Player) * players.size();
		send_buffer = std::make_unique<char*>(new char[data_size] {});
		char* buffer_begin = *send_buffer;

		for (auto& player : players)
		{
			if (player->name != (*new_player)->name)
			{
				SerializedPlayer sp = SerializedPlayer::player_serializer(*player);
				std::memcpy(*send_buffer, *sp.data, sp.size);
				*send_buffer += sp.size;
			}
		}
		*send_buffer = buffer_begin;

		size_t size = players.size();
		char size_str[10]{};
		std::strcpy(size_str, std::to_string(size).c_str());
		char temp[4]{};

		{ // sends the size of the incoming data in the next transmission. essentially letting the client know how many players' info are about to be sent
			send(ClientSockets[idx], size_str, std::strlen(size_str), 0);
			recv(ClientSockets[idx], temp, 3, 0);
		}
		
		send(ClientSockets[idx], *send_buffer, data_size, 0);
		++count;
	}
	cout << (*new_player)->name + ": communication finished..." << endl;
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