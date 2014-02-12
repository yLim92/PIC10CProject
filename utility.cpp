#include "utility.h"
#include<random>
#include<iostream>
#include<string>
#include <stdio.h>
#include <windows.h>
#include <exception>

using namespace std;

const std::string gc::combo_point_name(ComboPointType cpt){
	using namespace gc;
	if (cpt == ComboPointType::attack)
		return "ATK";
	else if (cpt == ComboPointType::dodge)
		return "DDG";
	else if (cpt == ComboPointType::disrupt)
		return "DSR";
	else
		return "";
}
//Returns true if any ComboPointType total from the left hand side is less than the corresponding ComboPointType total on the rhs
bool gc::operator<(const gc::ComboPoints& lhs, const gc::ComboPoints& rhs){
	gc::ComboPoints rh = rhs;
	for (auto it = lhs.begin(); it != lhs.end(); ++it){
		if (it->second < rh[it->first]){
			return true;
		}
	}
	return false;
}
gc::ComboPoints gc::operator-(const ComboPoints& rhs){
	gc::ComboPoints copy;
	for (auto it = rhs.begin(); it != rhs.end(); ++it){
		copy[it->first] = -it->second;
	}
	return copy;
}

int utility::rng(int n) {
	std::random_device rd;
	std::default_random_engine e1(rd());
	std::uniform_int_distribution<int> uniform_dist(1, n);
	return uniform_dist(e1);
}
int utility::rng(int l, int h) {
	std::random_device rd;
	std::default_random_engine e1(rd());
	std::uniform_int_distribution<int> uniform_dist(l, h);
	return uniform_dist(e1);
}

double utility::normal_rng(double mean, double std_dev){
	std::random_device rd;
	std::mt19937 e2(rd());
	std::normal_distribution<> normal_dist(mean, std_dev);
	return normal_dist(e2);
}

int utility::get_user_input(int h) {
	return utility::get_user_input(1, h);
}

int utility::get_user_input(int l, int h) {
	std::string user_input;
	std::getline(std::cin, user_input);
	bool valid = false;
	while (!valid) {
		try {
			if (std::stoi(user_input) < l || std::stoi(user_input) > h) 
				throw std::exception("Invalid input");
			else
				valid = true;
		}
		catch (std::exception&) {
			std::getline(std::cin, user_input);
		}
	}
	return std::stoi(user_input);
}

int utility::get_user_input(int h, std::vector<int> &ignore) {
	return get_user_input(1, h, ignore);
}

int utility::get_user_input(int l, int h, std::vector<int> &ignore) {
	std::string user_input;
	std::getline(std::cin, user_input);
	bool valid = false;
	while (!valid) {
		try {
			if (std::stoi(user_input) < l || std::stoi(user_input) > h ||
				std::find(ignore.begin(), ignore.end(), std::stoi(user_input)) != ignore.end())
				throw std::exception("Invalid input");
			else
				valid = true;
		}
		catch (std::exception&) {
			std::getline(std::cin, user_input);
		}
	}
	
	return std::stoi(user_input);
}


void view::clear_screen()
{
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X *csbi.dwSize.Y;

	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR) ' ',
		cellCount,
		homeCoords,
		&count
		)) return;

	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
		)) return;

	/* Move the cursor home */
	SetConsoleCursorPosition(hStdOut, homeCoords);
}

void view::cc(const char *out, int c) {
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, c);
	std::cout << out;
	SetConsoleTextAttribute(hConsole, view::dark_white);
}
void view::cc(const std::vector<const char*> &out, int c) {
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (size_t i = 0; i < out.size(); ++i) {
		SetConsoleTextAttribute(hConsole, c);
		std::cout << out[i];
	}

	SetConsoleTextAttribute(hConsole, view::dark_white);
}
void view::cc(const std::vector<const char*> &out, std::vector<int> c) {
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (size_t i = 0; i < out.size(); ++i){
		try {
			if (c.size() - 1 < i){
				std::exception e("Out of range");
				throw e;
			}
			SetConsoleTextAttribute(hConsole, c[i]);
		}
		catch (std::exception& ){
			SetConsoleTextAttribute(hConsole, view::solid_dark_red);
		}
		std::cout << out[i];
	}
	SetConsoleTextAttribute(hConsole, view::dark_white);
}

void view::ccn(const char *out, int c) {
	cc(out, c);
	std::cout << std::endl;
}
void view::ccn(const std::vector<const char*> &out, int c) {
	cc(out, c);
	std::cout << std::endl;
}
void view::ccn(const std::vector<const char*> &out, std::vector<int> c) {
	cc(out, c);
	std::cout << std::endl;
}
void view::cc(std::string out, int c){
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, c);
	std::cout << out;
	SetConsoleTextAttribute(hConsole, view::dark_white);
}
void view::ccn(std::string out, int c){
	cc(out, c);
	std::cout << std::endl;
}
void view::ccn(const std::vector<color_pair> &out) {
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (size_t i = 0; i < out.size(); ++i){
		SetConsoleTextAttribute(hConsole, out[i].second);
		std::cout << out[i].first;
	}
	SetConsoleTextAttribute(hConsole, view::dark_white);
	std::cout << std::endl;
}

const char* dir::cardinal(int d) {
	if (d == dir::North)
		return "North";
	else if (d == dir::East)
		return "East";
	else if (d == dir::South)
		return "South";
	else if (d == dir::West)
		return "West";
	return "";
}

