#ifndef UTILITY_H
#define UTILITY_H

#include <iostream>
#include <utility>
#include <string>
#include "windows.h"
#include <vector>
#include <ctime>
#include <array>
#include <map>

#define DEBUG_FLOOR 0
#define DEBUG_BATTLE 1
#define DEBUG_TARGET_LOGIC 0
#define DEBUG_STATUS 0

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::map;
using std::pair;

//Game constants
namespace gc {
	const int FPS = 10;
	const int AVG_FRAMES_PER_TURN = 50;
	const int PRACTICALLY_INFINITY = 100000;
	const int PERFECT_HIT_THRESHOLD = 100;

	enum class Threat {
		low = 20,
		mid = 40,
		high = 60,
	};
	enum class RoomCode {
		unvisited = 0,
		incomplete,
		success,
		boss_success,
		failure,
	};
	enum class Affiliation {
		player,
		enemy,
		ally,
		neutral
	};
	enum class StatusType {
		ability,
		damage_reduction,
		effect_chance,
		critical_chance,
		critical_damage,
		dodge,
		effect_resist,
		speed,
		accuracy,
		channelling,
		stun,
		grab,
		burn,
		bleed,
		poison,
		temp_hp_up,
		regen_temp_hp,
		regen_resource,
		armor_pierce,
		challenged,
		threat_level,
		ability_status_on_hit,
		ability_status_on_dmg,
		undefined
	};
	enum class ComboPointType {
		attack,
		dodge,
		disrupt,
	};
	const std::string combo_point_name(ComboPointType cpt);
	typedef std::map<ComboPointType, int> ComboPoints;
	bool operator<(const ComboPoints& lhs, const ComboPoints& rhs);
	ComboPoints operator-(const ComboPoints& rhs);

	enum class TargetType {
		self,
		single_friendly,
		single_enemy,
		party_friendly,
		party_enemy,
		all
	};
}


namespace utility {
	int rng(int n);
	int rng(int l, int h);
	double normal_rng(double mean, double std_dev);
	int get_user_input(int h);
	int get_user_input(int l, int h);
	int get_user_input(int h, std::vector<int> &ignore);
	int get_user_input(int l, int h, std::vector<int> &ignore);
	std::string base_32_hex();

	typedef std::pair<int, int> coordinates;
}

namespace options {
	enum Difficulty {
		easy = 0,
		normal,
		hard,
		absurd
	};
}
namespace dir {
	enum Direction {
		North,
		West,
		East,
		South
	};
	const char* cardinal(int d);
}

namespace view {
	
	//Color cout
	void clear_screen();
	void cc(const char *out, int c);
	void cc(const std::vector<const char*> &out, int c);
	void cc(const std::vector<const char*> &out, std::vector<int> c);
	//Color cout with newline
	void ccn(const char *out, int c);
	void ccn(const std::vector<const char*> &out, int c);
	void ccn(const std::vector<const char*> &out, std::vector<int> c);

	//Color cout with strings
	typedef std::pair<std::string, int> color_pair;
	void cc(std::string out, int c);
	void ccn(std::string out, int c);
	void ccn(const std::vector<color_pair> &out);

	enum Colors {
		black,
		dark_blue,
		dark_green,
		dark_teal,
		dark_red,
		dark_magenta,
		dark_yellow,
		dark_white,
		gray,
		blue,
		green,
		teal,
		red,
		magenta,
		yellow,
		white,
		solid_dark_blue = 17,
		solid_dark_green = 34,
		solid_dark_teal = 51,
		solid_dark_red = 68,
		solid_dark_magenta = 85,
		solid_dark_yellow = 102,
		solid_dark_white = 119,
		solid_gray = 136,
		solid_blue = 153,
		solid_green = 170,
		solid_teal = 187,
		solid_red = 204,
		solid_magenta = 221,
		solid_yellow = 238,
		solid_white = 255
	};

}



/*

clock_t begin = clock();
for (int i = 0; i < 1000000; i++) {
int q = 1;
}
std::cout <<  (((double)(clock() - begin)) / CLOCKS_PER_SEC) << " sec." << std::endl;

*/

#endif