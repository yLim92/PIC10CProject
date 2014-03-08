#include "events.h"
#include <iostream>
#include "layout.h"
#include "battlephase.h"
#include "gameunit.h"
#include <vector>

EventList::EventList() {
	set_possible_events();
}
void EventList::set_possible_events(){
	possible_events.push_back(new DracoZombieSummoningEvent());
}

void EventList::populate_event_table(Floor *f) {
	const std::vector<Room*> &r = f->get_rooms();
	for (size_t i = 0; i < r.size(); ++i){
		if (r[i]->get_coordinates() != f->get_starting_room().get_coordinates())
			event_table[r[i]->get_coordinates()] = possible_events[utility::rng(0, possible_events.size()-1)];
	}
	event_table[f->get_starting_room().get_coordinates()] = new Event();
}

gc::RoomCode EventList::do_room_event(utility::coordinates u, Party *p){
	return event_table[u]->do_event(p);
}

Event::~Event(){
}

gc::RoomCode Event::do_event(Party *p) {
	if (room_code == gc::RoomCode::unvisited){
		std::cout << "The party enters the room." << std::endl;
		room_code = gc::RoomCode::success;
	}
	else {
		std::cout << "The party enters the room again." << std::endl;
	}
	return room_code;
}

gc::RoomCode BattleEvent::do_event(Party *p) {
	if (room_code == gc::RoomCode::unvisited){
		std::vector<GameUnit*> e;
		for (int i = 0; i < 1; ++i)
			e.push_back(new DracoZombie(10));
		Party *enemy_party = new Party(e,gc::Affiliation::enemy);
		BattlePhase b(p, enemy_party);
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

/* IN THE FUTURE 
	-Need to add a wait function
	-Handle transitions between battles (battlephase was never deleted until it was reassigned...)

*/
gc::RoomCode DracoZombieSummoningEvent::do_event(Party *p) {
	if (room_code == gc::RoomCode::unvisited){
		view::clear_screen();

		string wait = "";
		std::cout << "The party enters the room." << std::endl;
		getline(std::cin, wait);
		cout << "A foul ritual is taking place.  ";
		cout << "Cultists, eager to appease whatever unholy god they worship, are sacrificing human prisoners to fuel a dark incantation." << endl;
		getline(std::cin, wait);
		cout << "Judging from the size of the summoning portal, there isn't much time." << endl;
		getline(std::cin, wait);
		cout << "You could try to save some of the remaining prisoners..." << endl;
		cout << "Or leave them to their fate and attempt to disrupt the ritual." << endl;
		getline(std::cin, wait);

		cout << "What should the party do??" << endl;
		cout << "1. Save the sacrifices!" << endl;
		cout << "2. Disrupt the ritual!" << endl;
		cout << "3. Unholy ritual? Cool. I want to see what happens." << endl;

		int ui = utility::get_user_input(3);

		if (ui == 1){
			cout << "The party rushes to the sacrificial grounds." << endl;
			getline(std::cin, wait);
			cout << "The pair of cultists conducting the sacrifices are prepared." << endl;
			cout << "The soul-deprived husks of their victims rise, unwilling but unable to resist the call of their new masters." << endl;
			getline(std::cin, wait);
			cout << "The party has no choice but to cut down these hollows and their puppeteers." << endl;
			getline(std::cin, wait);
			vector<GameUnit*> e;
			for (int i = 0; i < 1; ++i)
				e.push_back(new Cultist(1));
			for (int i = 0; i < 1; ++i)
				e.push_back(new Fallen(1));
			Party *enemy_party = new Party(e,gc::Affiliation::enemy);
			BattlePhase * first_battle = new BattlePhase(p, enemy_party);

			first_battle->do_battle();
			delete first_battle;

			view::clear_screen();
			cout << "As the party strikes down the last enemy, a terrible roar is heard." << endl;
			getline(std::cin, wait);
			cout << "The result of the ritual becomes terribly clear: a Draco-Zombie, a decaying relic of a forgotten age." << endl;
			getline(std::cin, wait);
			cout << "It promptly devours the summoners and turns its eyeless sockets towards the party." << endl;
			getline(std::cin, wait);
			e.clear();
			for (int i = 0; i < 1; ++i)
				e.push_back(new DracoZombie(10));
			Party* dz_party = new Party(e,gc::Affiliation::enemy);
			BattlePhase * second_battle = new BattlePhase(p, dz_party);
			second_battle->do_battle();
		}
		else if (ui == 2){
			cout << "The party leaves the prisoners to their fate and confront the cultists." << endl;
			getline(std::cin, wait);
			vector<GameUnit*> e;
			for (int i = 0; i < 4; ++i)
				e.push_back(new Cultist(10));
			Party *enemy_party = new Party(e,gc::Affiliation::enemy);
			BattlePhase * first_battle = new BattlePhase(p, enemy_party);
			first_battle->do_battle();
			delete first_battle;


			view::clear_screen();
			cout << "It's too late!  Even with the cultists dead, the horrifying product of the ritual emerges!" << endl;
			getline(std::cin, wait);
			cout << "A Draco-Zombie." << endl;
			cout << "The party steels itself." << endl;
			getline(std::cin, wait);

			e.clear();
			for (int i = 0; i < 1; ++i)
				e.push_back(new DracoZombie(7));
			Party* dz_party = new Party(e,gc::Affiliation::enemy);
			BattlePhase * second_battle = new BattlePhase(p, dz_party);
			second_battle->do_battle();
		}
		else if (ui == 3){
			cout << "The party waits for the ritual to be complete." << endl;
			cout << "A Draco-Zombie is summoned, which promptly devours everything around it...and turns to the party." << endl;
			getline(std::cin, wait);
			vector<GameUnit*> e;
			for (int i = 0; i < 1; ++i)
				e.push_back(new DracoZombie(12));
			Party* dz_party = new Party(e,gc::Affiliation::enemy);
			BattlePhase * second_battle = new BattlePhase(p, dz_party);
		}



		/*
		std::vector<GameUnit*> e;
		for (int i = 0; i < 1; ++i)
			e.push_back(new DracoZombie(10));
		Party *enemy_party = new Party(e,gc::Affiliation::enemy);
		BattlePhase b(p, enemy_party);

		
		b.do_battle();*/

		room_code = gc::RoomCode::success;
	}
	else {
		std::cout << "You enter the room again." << std::endl;
	}
	return room_code;
}