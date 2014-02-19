#ifndef BATTLEPHASE_H
#define BATTLEPHASE_H

#include <vector>
#include <map>
#include "utility.h" //affiliation


#include<iostream>

class GameUnit;
class Party;
class Turn;
struct DelayedAbility;
class BattleView;
class Ability;
class StatusBlock;
class Status;

class BattlePhase {
	friend class BattleView;
public:
	BattlePhase();
	BattlePhase(Party* pp, Party* ep);
	BattlePhase(Party* pp, Party* ep, Party* ap, Party* np);
	~BattlePhase();

	int do_battle();

	bool is_battle_complete();

	const Party& get_player_party() const { return *player_party; }
	const Party& get_enemy_party() const { return *enemy_party; }
	const Party& get_ally_party() const { return *ally_party; }
	const Party& get_neutral_party() const { return *neutral_party; }

	BattleView& get_battle_view() { return *bv; }
	std::vector<GameUnit*>& get_combatants() { return combatants; }
private:
	Party* player_party;
	Party* enemy_party;
	Party* ally_party;
	Party* neutral_party;

	//std::map<GameUnit*, Turn*> gu_turn;

	std::vector<GameUnit*> combatants;
	

	BattleView* bv;
};

class BattleView {
public:
	BattleView(BattlePhase *b);
	~BattleView() {}

	void print_battle_view();
	void print_combatants();
	void print_battle_log();
	void print_menus();
	void add_to_battle_log(std::string s, int c);
	void add_to_battle_log(const std::vector<std::pair<std::string, int> > &vp);
private:
	std::vector<std::vector<std::pair<std::string, int> > > battle_log;
	BattlePhase* bp;

	int total_width;
	int column_width;
	int padding;

	//Stuff to render view
	enum Stat {
		name,
		health,
		//damage,
		temp_hp,
		//temp_hp_remainder,
		turn_progress,
		//turn_total
		stat_eff
	};
	//typedef std::map<Stat, std::pair<std::string, view::Colors> > blockfield;
	typedef std::map<Stat, std::vector<std::pair<std::string, view::Colors> > > blockfield;

	void add_blockfield(std::vector<blockfield> &bf_v, const GameUnit &gu);
	void pad();
	void next_col(int n);
	std::string status_shorthand(gc::StatusType st);

	void print_status_bars(blockfield &bf, int start, int limit);
	void print_health_bars(blockfield &bf);
	void print_turn_bars(blockfield &bf);
};
/*
class Turn {
	friend class BattleView;
public:
	Turn();
	Turn(BattlePhase *b, GameUnit *g);
	virtual ~Turn() {}

	virtual int do_turn(BattleView &bv);
protected:
	BattlePhase *bp;
	GameUnit *gu;

};

class PlayerTurn: public Turn {    
public:
	PlayerTurn() {};
	PlayerTurn(BattlePhase *b, GameUnit *g) : Turn(b, g) {}
	virtual ~PlayerTurn() {}

	virtual int do_turn(BattleView &bv);
	
	virtual void list_abilities(BattleView &bv);
};

class NpcTurn : public Turn {
public:
	NpcTurn();
	NpcTurn(BattlePhase *b, GameUnit *g) : Turn(b, g) {}
	virtual ~NpcTurn() {}

	virtual int do_turn(BattleView &bv);
};*/

#endif