#include <iostream>

#include "explorephase.h"
#include "layout.h"
#include "utility.h"
#include "gameunit.h"

#include <time.h>

ExplorePhase::ExplorePhase(){
}

ExplorePhase::ExplorePhase(Difficulty d, Party *p) : difficulty(d), party(p) {
	event_list = EventList();
	add_new_floor();
}

void ExplorePhase:: explore() {
	while (event_list.do_room_event(current_floor->current_room->get_coordinates()) != gc::RoomCode::boss_success) {
		post_event_phase();
	}
}

void ExplorePhase::add_new_floor() {
	Floor *f = new Floor(8, 8, 0.6);
	floors.push_back(f);
	current_floor = f;
	event_list.populate_event_table(f, party);
}

void ExplorePhase::post_event_phase() {
	std::array<Room*, 4>& adj_rooms = current_floor->current_room->adjacent_rooms;
	std::vector<int> ignore;

	view::clear_screen();
	current_floor->print_floor();
	view::cc("You are in ", view::white);
	current_floor->current_room->print_room_info();

	for (size_t i = 0; i < adj_rooms.size(); ++i) {
		std::string i_1 = std::to_string(i + 1);
		std::array<const char*, 3> text_init = { i_1.c_str(), ". Go ", dir::cardinal(i) }; std::vector<const char*> text(text_init.begin(), text_init.end());
		if (adj_rooms[i] != nullptr) 
			view::ccn(text, view::white);
		else {
			view::ccn(text, view::gray);
			ignore.push_back(i + 1);
		}
	}

	int user_input = utility::get_user_input(4, ignore);
	if (user_input <= 4){
		current_floor->set_current_room(adj_rooms[user_input-1]);
	}
}