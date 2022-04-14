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
#include <thread>
#include <vector>
#include "Player.h"
#include "SerializedPlayer.h"

using namespace sdds;
using std::cout;
using std::cin;
using std::endl;
using std::string;

const string CLIENT_NAME = "Alireza";
std::vector<Player*> other_players = std::vector<Player*>();

void main()
{
	// starts Winsock DLLs
	WSADATA wsaData;
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
		return;

	// initializes socket. SOCK_STREAM: TCP
	SOCKET ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ClientSocket == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}

	// Connect socket to specified server
	sockaddr_in SvrAddr{};
	SvrAddr.sin_family = AF_INET;						// Address family type itnernet
	SvrAddr.sin_port = htons(27000);					// port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	// IP address
	if ((connect(ClientSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR)
	{
		closesocket(ClientSocket);
		WSACleanup();
		return;
	}

	int count = 0;
	Player this_player(Location(), CLIENT_NAME);
	char* recv_buffer{};
	float x{}, y{}, z{};

	while (count < 15)
	{
		x = rand_float(0, 10);
		y = rand_float(0, 10);
		z = rand_float(0, 10);
		this_player.location.x = x; this_player.location.y = y; this_player.location.z = z;
		SerializedPlayer sp = SerializedPlayer::player_serializer(this_player);
		send(ClientSocket, *sp.data, sp.size, 0);
		char s[1]{};
		recv(ClientSocket, s, 1, 0);
		size_t data_size = std::strtol(s, NULL, 10);
		send(ClientSocket, "ack", 3, 0);
		std::unique_ptr<char*> recv_buffer = std::make_unique<char*>(new char[data_size * sizeof(Player)]{});
		recv(ClientSocket, *recv_buffer, data_size * sizeof(Player), 0);
		for (int i = 0; i < data_size; i++)
		{
			if (std::strlen(*recv_buffer) > 0)
			{
				Player recv_player = SerializedPlayer::player_deserializer(*recv_buffer);
				auto& const name = recv_player.name;
				auto found = std::find_if(other_players.begin(), other_players.end(), [&name](const Player* player)
					{
						return name == player->name;
					});
				if (found != other_players.cend())
				{
					(**found).location.update_loc(recv_player.location.x, recv_player.location.y, recv_player.location.z);
					cout << std::left << std::setw(3) << count << " - ";
					printPlayerInfo(*found);
				}
				else
					other_players.push_back(&recv_player);

				*recv_buffer += sizeof(Player);
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
		++count;
	}
	cout << "End of transmission. Terminating..." << endl;
	// closes connection and socket
	closesocket(ClientSocket);
	// frees Winsock DLL resources
	WSACleanup();
}