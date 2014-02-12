#include "layout.h"

#include<string>
#include<ctime>
#include<vector>
#include"utility.h"


using namespace std;


Room::Room() : x(0), y(0), explored(false) {
}

Room::Room(int a, int b) : x(a), y(b), explored(false) {
	for (int i = 0; i < 4; ++i)
		adjacent_rooms[i] = nullptr;
}

void Room::connect_room(Room *r, int c) {
	this->adjacent_rooms[c] = r;
	if (c == dir::North)
		r->adjacent_rooms[dir::South] = this;
	else if (c == dir::West)
		r->adjacent_rooms[dir::East] = this;
	else if (c == dir::East)
		r->adjacent_rooms[dir::West] = this;
	else if (c == dir::South)
		r->adjacent_rooms[dir::North] = this;
	/*
	int d = (c + 2 < 4) ? c + 2 : c - 2;
	r->adjacent_rooms[d] = this;*/

}
const void Room::print_room_info() const {
	cout << "Room(" << x << "," << y << ")" << endl;
}
const void Room::print_adjacent_rooms() {
	string dir[4] = {
		"North",
		"East",
		"South",
		"West"
	};
	for (unsigned int i = 0; i < adjacent_rooms.size(); ++i){
		if (adjacent_rooms[i] != nullptr) {
			cout << dir::cardinal(i) << ": Room(" << adjacent_rooms[i]->get_x() << "," << adjacent_rooms[i]->get_y() << ")" << endl;
		}
	}
}

int Room::get_connections() {
	int c = 0;
	for (int i = 0; i < (int)(adjacent_rooms.size()); ++i)
	if (adjacent_rooms[i] != nullptr)
		c++;
	return c;
}
const pair<int, int> Room::get_coordinates() const {
	return std::make_pair(get_x(), get_y());
}



Floor::Floor() : x(1), y(1), room_density(0.6) {
}

Floor::Floor(int a, int b) : x(a), y(b), room_density(0.6) {
}

Floor::Floor(int a, int b, double rd) : x(a), y(b), room_density(rd) {

	generate_floor();
	if (DEBUG_FLOOR)
		debug_floor_info();

}

void Floor::generate_floor() {
	std::srand(unsigned int(time(NULL)));

	int a = utility::rng(x) - 1;
	int b = utility::rng(y) - 1;
	starting_room = new Room(a, b);
	floor_map[a][b] = starting_room;
	set_current_room(starting_room);

	int i = 1;
	int limit = (int)(x*y*room_density);
	vector<Room*> rooms;
	rooms.push_back(starting_room);
	while (i < limit) {
		int dir[4] = { dir::North, dir::East, dir::South, dir::West };
		random_shuffle(std::begin(dir), std::end(dir));
		int rm = utility::rng((int)(rooms.size())) - 1;

		int j = 0;
		bool isfeasible = false;
		while (j < 4) {
			int x_coor = rooms[rm]->get_x();
			int y_coor = rooms[rm]->get_y();
			if (dir[j] == dir::North)
				y_coor--;
			else if (dir[j] == dir::East)
				x_coor++;
			else if (dir[j] == dir::South)
				y_coor++;
			else if (dir[j] == dir::West)
				x_coor--;

			if (viable_room(x_coor, y_coor)){
				//-1 to account for the given adjancey
				isfeasible = true;
				if ((get_adj_room_count(x_coor, y_coor) - 1) / 2.0 < abs(utility::normal_rng(0, 0.5))) {
					Room *r = new Room(x_coor, y_coor);
					floor_map[x_coor][y_coor] = r;
					rooms[rm]->connect_room(r, dir[j]);
					rooms.push_back(r);
					i++;
					break;
				}
			}
			j++;
			if (j == 4 && !isfeasible) {
				rooms.erase(rooms.begin() + rm);
			}
		}
	}
}

void Floor::print_floor() {
	using namespace view;

	for (int i = 0; i < y; ++i){
		for (int k = 0; k < 6; ++k){
			for (int j = 0; j < x; ++j){
				const int vd = view::black;
				if (floor_map[j][i] == nullptr)
					view::cc("         ", vd);
				else {
					const int wl = get_room_color(floor_map[j][i]);
					const int fl = view::black;
					if (k == 0) {
						if (k == 0 && floor_map[j][i]->adjacent_rooms[dir::North] != nullptr){
							cc("   ",vd);cc("X",wl);cc(" ",fl);cc("X",wl);cc("   ",vd);
						}
						else
							view::cc("         ", vd);
					}
					else if (k == 1 || k == 5) {
						if ((k == 1 && floor_map[j][i]->adjacent_rooms[dir::North] != nullptr)
							|| (k == 5 && floor_map[j][i]->adjacent_rooms[dir::South] != nullptr)) {
							cc(" ",vd);cc("XXX",wl);cc(" ",fl);cc("XXX",wl);cc(" ",vd);
						}
						else{
							cc(" ",vd);cc("XXXXXXX",wl);cc(" ",vd);
						}
					}
					else if (k == 2 || k == 4) {
						if (floor_map[j][i]->adjacent_rooms[dir::West] != nullptr){
							cc("XX",wl);cc("     ",fl);
						}
						else {
							cc(" ",vd);cc("X",wl);cc("     ",fl);
						}
						if (floor_map[j][i]->adjacent_rooms[dir::East] != nullptr)
							cc("XX", wl);
						else{
							cc("X",wl);cc(" ",vd);
						}
					}
					else if (k == 3) {
						if (floor_map[j][i]->adjacent_rooms[dir::West] != nullptr)
							cc("    ", fl);
						else{
							cc(" ",vd);cc("X",wl);cc("  ",fl);
						}
						if (floor_map[j][i] == starting_room)
							cc("S", view::white);
						else
							cc(" ", fl);
						if (floor_map[j][i]->adjacent_rooms[dir::East] != nullptr)
							cc("    ", fl);
						else{
							cc("  ",fl);cc("X",wl);cc(" ",vd);
						}
					}
					else{
						cc(" ",vd);cc("XXXXXXX",wl);cc(" ",vd);
					}
				}
			}
			cout << endl;
		}
	}
}

void Floor::debug_floor_info() {
	map<int, int> c_total;

	double times = 1000;

	for (int i = 0; i < (int)(times); ++i) {

		floor_map.clear();
		generate_floor();

		for (auto outer_it = floor_map.begin(); outer_it != floor_map.end(); ++outer_it) {
			for (auto inner_it = outer_it->second.begin(); inner_it != outer_it->second.end(); ++inner_it) {
				if (inner_it->second != nullptr)
					c_total[inner_it->second->get_connections()]++;
			}
		}

	}
	cout << c_total[1] / times << endl;
	cout << c_total[2] / times << endl;
	cout << c_total[3] / times << endl;
	cout << c_total[4] / times << endl;
}

bool Floor::viable_room(int a, int b) {
	if (a < 0 || a == x || b < 0 || b == y)
		return false;
	else if (floor_map[a][b] != nullptr)
		return false;
	return true;
}
int Floor::get_adj_room_count(int a, int b) {
	int count = 0;
	for (int i = -1; i < 2; i += 2) {
		if (!viable_room(a, b + i))
			count++;
	}
	for (int i = -1; i < 2; i += 2) {
		if (!viable_room(a + i, b))
			count++;
	}
	return count;
}

const std::vector<Room*> Floor::get_rooms() const {
	vector<Room*> r;
	for (auto outer_it = floor_map.begin(); outer_it != floor_map.end(); ++outer_it) {
		for (auto inner_it = outer_it->second.begin(); inner_it != outer_it->second.end(); ++inner_it) {
			if (inner_it->second != nullptr)
				r.push_back(inner_it->second);
		}
	}
	return r;
}

void Floor::set_current_room(Room* r){
	current_room = r;
	current_room->explored = true;
}

const int Floor::get_room_color(Room* r) const {
	if (r == current_room)
		return view::solid_green;
	else if (r->explored)
		return view::solid_white;
	else
		return view::solid_gray;
}