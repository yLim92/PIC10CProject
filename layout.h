#include<iostream>
#include<map>
#include<utility>
#include<vector>
#include<array>

class Room {
	friend class Floor;
	friend class ExplorePhase;
public:
	Room();
	Room(int a, int b);
	~Room() {}

	void connect_room(Room *r, int c);

	const void print_room_info() const;
	const void print_adjacent_rooms();

	const int get_x() const { return x; }
	const int get_y() const { return y; }
	const std::pair<int, int> get_coordinates() const;
	int get_connections();
private:
	int x;
	int y;
	bool explored;
	std::array<Room*,4> adjacent_rooms;
};


class Floor {
	friend class ExplorePhase;
public:
	Floor();
	Floor(int a, int b);
	Floor(int a, int b, double rd);

	void generate_floor();
	void print_floor();

	void debug_floor_info();

	bool viable_room(int a, int b);
	int get_adj_room_count(int a, int b);

	const std::vector< Room*> get_rooms() const;
	const Room&get_current_room() const { return *current_room; };
	const int get_room_color(Room* r) const;
	void set_current_room(Room *r); 

	~Floor() {}

private:
	std::map<int, std::map<int, Room*>> floor_map;
	int x;
	int y;
	double room_density;
	Room *starting_room;
	Room *current_room;
};
