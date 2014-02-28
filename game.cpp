#include<iostream>
#include "game.h"
#include "explorephase.h"
#include "utility.h" //Used for affiliation
#include "gameunit.h"

Game::Game() {

}

void Game::start() {
	intro();
}
void Game::intro() {
	cout << "Welcome to Delver!" << endl;
	cout << "1. New Game" << endl;
	cout << "2. Continue" << endl;
	
	int user_input = utility::get_user_input(2);
	if (user_input == 1) {
		new_game();
	}
	else if (user_input == 2) {
		continue_game();
	}
}
void Game::new_game() {
	cout << "Select game difficulty:" << endl;
	cout << "1. Easy" << endl;
	cout << "2. Normal" << endl;
	cout << "3. Hard" << endl;
	cout << "4. Absurd" << endl;

	Difficulty difficulty = (Difficulty)(utility::get_user_input(4));

	//cout << "Create your party:" << endl;

	std::vector<GameUnit*> players;
	players.push_back(new Soldier("Fred", 10));
	players.push_back(new Mage("Jim", 10));
	players.push_back(new Soldier("Newbie", 1));
	players.push_back(new Mage("NewbieJr", 1));
	Party *party = new Party(players, gc::Affiliation::player);

	explorePhase = new ExplorePhase(difficulty, party);

	explorePhase->explore();
}
void Game::continue_game() {
	cout << "TO BE IMPLEMENTED" << endl;
}