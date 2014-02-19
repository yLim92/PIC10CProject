#include "events.h"
#include <iostream>
#include "layout.h"
#include "battlephase.h"
#include "gameunit.h"
#include <vector>

EventList::EventList() {

}

void EventList::populate_event_table(Floor *f, Party *p) {
	const std::vector<Room*> &r = f->get_rooms();
	for (size_t i = 0; i < r.size(); ++i){
		if (r[i]->get_coordinates() == f->get_current_room().get_coordinates())
			event_table[r[i]->get_coordinates()] = new BattleEvent(p);
		else
			event_table[r[i]->get_coordinates()] = new Event();
	}
}

gc::RoomCode EventList::do_room_event(utility::coordinates u){
	return event_table[u]->do_event();
}

Event::Event() {
	room_code = gc::RoomCode::unvisited;
}


Event::~Event(){
}

gc::RoomCode Event::do_event() {
	if (room_code == gc::RoomCode::unvisited){
		std::cout << "You enter the room." << std::endl;
		room_code = gc::RoomCode::success;
	}
	else {
		std::cout << "You enter the room again." << std::endl;
	}
	return room_code;
}

gc::RoomCode BattleEvent::do_event() {
	if (room_code == gc::RoomCode::unvisited){
		std::vector<GameUnit*> e;
		for (int i = 0; i < 2; ++i)
			e.push_back(new Npc("Bob", 10));
		Party *enemy_party = new Party(e,gc::Affiliation::enemy);
		BattlePhase b(party, enemy_party);
		std::cout << "You enter the room." << std::endl;
		std::cout << "BATTLE!" << std::endl;
		b.do_battle();

		room_code = gc::RoomCode::success;
	}
	else {
		std::cout << "You enter the room again." << std::endl;
	}
	return room_code;
}