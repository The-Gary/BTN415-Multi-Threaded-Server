#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <random>

struct Location
{
	float x{};
	float y{};
	float z{};
	void update_loc(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

struct SerializedPlayer
{
	char* data{};
	int size{};
};

struct Player
{
	Location location{};
	std::string name{};

	Player() {};

	Player(Location loc, std::string name)
	{
		this->location = loc;
		this->name = name;
	}

	void print() const
	{
		std::cout  << "Coordinates for [" << std::setw(10) << this->name << "] are [X: " << std::setprecision(2) << std::fixed;
		std::cout << std::setw(4) << this->location.x << " | Y: ";
		std::cout << std::setw(4) << this->location.y << " | Z: ";
		std::cout << std::setw(4) << this->location.z << "]" << std::endl;
	}
};

SerializedPlayer& player_serializer(const Player& player)
{
	SerializedPlayer sp{};
	sp.size = sizeof(Player);
	size_t name_size = player.name.length();
	sp.data = new char[sp.size];
	char* auxptr = sp.data;
	memcpy(auxptr, &name_size, sizeof(size_t));
	auxptr += sizeof(size_t);
	memcpy(auxptr, player.name.c_str(), player.name.size());
	auxptr += player.name.size();
	memcpy(auxptr, &player.location, sizeof(Location));
	return sp;
};

Player player_deserializer(const char* auxptr)
{
	Player player{};
	size_t name_size = 0;
	memcpy(&name_size, auxptr, sizeof(size_t));
	auxptr += sizeof(size_t);
	char* name = new char[name_size];
	memcpy(name, auxptr, name_size);
	auto s_name = std::string(name, name_size);
	player.name = s_name;
	auxptr += name_size;
	memcpy(&player.location, auxptr, sizeof(Location));
	delete[] name;
	return player;
};


float rand_float(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> distr(min, max);
	return distr(gen);
}

#endif // !PLAYER_H
