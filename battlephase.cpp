#include<iostream>
#include<exception>
#include <iomanip> 

#include "battlephase.h"
#include "gameunit.h"
#include "abilities.h"

#include "utility.h"

BattlePhase::BattlePhase() {
	
}

BattlePhase::~BattlePhase() {
	//delete player_party;
	delete enemy_party;
	delete ally_party;
	delete neutral_party;
}

BattlePhase::BattlePhase(Party* pp, Party* ep) : player_party(pp), enemy_party(ep) {
	ally_party = new Party();
	neutral_party = new Party();

	for (size_t i = 0; i < player_party->get_party_members().size(); i++){
		//gu_turn[player_party->get_party_members()[i]] = new PlayerTurn(this, player_party->get_party_members()[i]);
		combatants.push_back(player_party->get_party_members()[i]);
	}
	for (size_t i = 0; i < enemy_party->get_party_members().size(); i++){
		//gu_turn[enemy_party->get_party_members()[i]] = new NpcTurn(this, enemy_party->get_party_members()[i]);
		combatants.push_back(enemy_party->get_party_members()[i]);
	}
	bv = new BattleView(this);
}

//Parties passed it must not have the same affiliation
BattlePhase::BattlePhase(Party* pp, Party* ep, Party* ap, Party* np) : player_party(pp), enemy_party(ep), ally_party(ap), neutral_party(np) {
	
	if (player_party == nullptr)
		player_party = new Party();
	if (enemy_party == nullptr)
		enemy_party = new Party();
	if (ally_party == nullptr)
		ally_party = new Party();
	if (neutral_party == nullptr)
		neutral_party = new Party();

	//can sort combatants by speed, then fill gu_turn map with sorted pairs to solve "ties"
	for (size_t i = 0; i < player_party->get_party_members().size(); i++){
		//gu_turn[player_party->get_party_members()[i]] = new PlayerTurn(this, player_party->get_party_members()[i]);
		combatants.push_back(player_party->get_party_members()[i]);
	}
	for (size_t i = 0; i < enemy_party->get_party_members().size(); i++){
		//gu_turn[enemy_party->get_party_members()[i]] = new NpcTurn(this, enemy_party->get_party_members()[i]);
		combatants.push_back(enemy_party->get_party_members()[i]);
	}
	for (size_t i = 0; i < ally_party->get_party_members().size(); i++){
		//gu_turn[ally_party->get_party_members()[i]] = new NpcTurn(this, ally_party->get_party_members()[i]);
		combatants.push_back(ally_party->get_party_members()[i]);
	}
	for (size_t i = 0; i < neutral_party->get_party_members().size(); i++){
		//gu_turn[neutral_party->get_party_members()[i]] = new NpcTurn(this, neutral_party->get_party_members()[i]);
		combatants.push_back(neutral_party->get_party_members()[i]);
	}
	

	bv = new BattleView(this);
}

int BattlePhase::do_battle() {
	while (!is_battle_complete()){
		bv->print_battle_view();
		clock_t begin = clock();
		while ((((double)(clock() - begin)) / CLOCKS_PER_SEC) < 1.0/gc::FPS){}
		for (size_t i = 0; i < combatants.size(); ++i){
			if (combatants[i]->can_take_turn() && combatants[i]->is_turn())
				combatants[i]->do_turn(combatants, *bv);
		}
		for (size_t i = 0; i < combatants.size(); ++i){
			combatants[i]->update(*bv);
		}
	}
	return 0;
}

bool BattlePhase::is_battle_complete() {
	if (player_party->is_party_defeated())
		return true;
	else if (enemy_party->is_party_defeated() && neutral_party->is_party_defeated()) 
		return true;
	return false;
}

BattleView::BattleView(BattlePhase *b) : bp(b){
	total_width = 90;
	column_width = 45;
	padding = 1;
	battle_log.resize(20);
}
void BattleView::pad() {
	std::cout << std::setw(padding); view::cc("X", view::black);
}

void BattleView::next_col(int n) {
	std::cout << std::setw(column_width - n); view::cc("X", view::black); view::cc("XX", view::black); 
}
void BattleView::add_blockfield(std::vector<BattleView::blockfield> &bf_v, const GameUnit &gu) {
	using namespace std;
	using namespace gc;

	blockfield bf;
	string nm = gu.get_name();
	string hlth = "";
	string dmg = "";
	string tmphp = "";
	string tmphpr = "";
	string tn = "";
	string tn_tot = "";

	for (int i = 0; i < 20; ++i){
		if (gu.get_current_hp() * 20 / gu.get_max_hp() > i) hlth += "X";
		else dmg += "X";
	}
	for (int i = 0; i < 20; ++i){
		if (gu.get_temp_hp() * 20 / (gu.get_max_hp()) > i) tmphp += "X";
		else tmphpr += "X";
	}
	for (int i = 0; i < 20; ++i){
		if (int(gu.get_turn_progress()*20) > i) tn += "X";
		else tn_tot += "X";
	}

	bf[name].push_back(make_pair(nm, view::white));
	bf[health].push_back(make_pair(hlth, view::solid_green));
	bf[health].push_back(make_pair(dmg, view::solid_red));
	bf[temp_hp].push_back(make_pair(tmphp, view::solid_teal));
	bf[temp_hp].push_back(make_pair(tmphpr, view::black));
	bf[turn_progress].push_back(make_pair(tn, view::solid_white));
	bf[turn_progress].push_back(make_pair(tn_tot, view::solid_gray));
	
	map<gc::StatusType, double> sm = gu.get_all_status_and_values();
	for (auto it = sm.begin(); it != sm.end(); ++it){
		view::Colors stat_color;
		if (it->first == StatusType::ability || 
			it->first == StatusType::damage_reduction ||
			it->first == StatusType::armor_pierce ||
			it->first == StatusType::effect_chance ||
			it->first == StatusType::effect_resist ||
			it->first == StatusType::critical_chance ||
			it->first == StatusType::critical_damage ||
			it->first == StatusType::dodge ||
			it->first == StatusType::speed ||
			it->first == StatusType::accuracy ||
			it->first == StatusType::temp_hp_up ||
			it->first == StatusType::regen_temp_hp ||
			it->first == StatusType::regen_resource){
			if (it->second >= 0) stat_color = view::green;
			else stat_color = view::red;
		}
		else if (it->first == gc::StatusType::channelling ||
			it->first == gc::StatusType::stun ||
			it->first == gc::StatusType::grab ||
			it->first == gc::StatusType::burn ||
			it->first == gc::StatusType::bleed ||
			it->first == gc::StatusType::poison ||
			it->first == gc::StatusType::challenged){
			stat_color = view::red;
		}
		else if (it->first == gc::StatusType::ability_status_on_dmg ||
				 it->first == gc::StatusType::ability_status_on_hit)
			stat_color = view::green;
		else if (it->first == gc::StatusType::threat_level){
			if (it->second >= 0) stat_color = view::red;
			else stat_color = view::green;
		}

		bf[stat_eff].push_back(make_pair(status_shorthand(it->first), stat_color));
	}
	
	bf_v.push_back(bf);
}
std::string BattleView::status_shorthand(gc::StatusType st){
	if (st == gc::StatusType::ability)				return "Abl ";
	else if (st == gc::StatusType::damage_reduction)return "Def ";
	else if (st == gc::StatusType::armor_pierce)	return "ArP ";
	else if (st == gc::StatusType::effect_chance)	return "EfC ";
	else if (st == gc::StatusType::effect_resist)	return "EfR ";
	else if (st == gc::StatusType::critical_chance) return "CrC ";
	else if (st == gc::StatusType::critical_damage) return "CrD ";
	else if (st == gc::StatusType::dodge)			return "Ddg ";
	else if (st == gc::StatusType::speed)			return "Spd ";
	else if (st == gc::StatusType::accuracy)		return "Acc ";
	else if (st == gc::StatusType::channelling)		return "Cha ";
	else if (st == gc::StatusType::stun)			return "Stn ";
	else if (st == gc::StatusType::grab)			return "Grb ";
	else if (st == gc::StatusType::burn)			return "Brn ";
	else if (st == gc::StatusType::bleed)			return "Bld ";
	else if (st == gc::StatusType::poison)			return "Psn ";
	else if (st == gc::StatusType::temp_hp_up)		return "Shd ";
	else if (st == gc::StatusType::regen_temp_hp)	return "RTH ";
	else if (st == gc::StatusType::regen_resource)	return "ReR ";
	else if (st == gc::StatusType::challenged)		return "Chl ";
	else if (st	== gc::StatusType::threat_level)	return "ThL ";
	else if (st	== gc::StatusType::ability_status_on_dmg)	return "OnD ";
	else if (st	== gc::StatusType::ability_status_on_hit)	return "OnH ";
	else return "";
}

void BattleView::print_health_bars(BattleView::blockfield &bf){
	pad(); cc("HP: ", view::green);
	cc(bf[health][0].first.c_str(), bf[health][0].second);
	cc(bf[health][1].first.c_str(), bf[health][1].second);
	cc(bf[temp_hp][0].first.c_str(), bf[temp_hp][0].second);
	cc(bf[temp_hp][1].first.c_str(), bf[temp_hp][1].second);
}
void BattleView::print_turn_bars(BattleView::blockfield &bf){
	pad(); cc("TN: ", view::white);
	cc(bf[turn_progress][0].first.c_str(), bf[turn_progress][0].second);
	cc(bf[turn_progress][1].first.c_str(), bf[turn_progress][1].second);
}
void BattleView::print_status_bars(BattleView::blockfield &bf, int start,int limit){
	for (int i = start; i < limit + start; ++i){
		if (int(bf[stat_eff].size()) > i)
			cc(bf[stat_eff][i].first.c_str(), bf[stat_eff][i].second);
		else
			cc("", view::black);
	}
}

void BattleView::print_battle_view(){
	print_combatants();
	print_battle_log();
	print_menus();
}



/* Generates the view for battles
	@pp,ep,ap,np are the vectors for the player, enemy, ally, and neutral party members
	@int block_count is the max number of blocks on either size
	@lb, rb are the left and right block vectors
*/
void BattleView::print_combatants(){
	using namespace view;
	using namespace std;

	vector<GameUnit *> pp = bp->get_player_party().get_party_members();
	vector<GameUnit *> ep = bp->get_enemy_party().get_party_members();
	vector<GameUnit *> ap = bp->get_ally_party().get_party_members();
	vector<GameUnit *> np = bp->get_neutral_party().get_party_members();

	const int block_count = int(pp.size() + ap.size()) > int(ep.size() + np.size()) ? int(pp.size() + ap.size()) : int(ep.size() + np.size()); 

	vector<blockfield> lb;
	vector<blockfield> rb;

	for (size_t i = 0; i < pp.size(); ++i)
		add_blockfield(lb, *pp[i]);
	for (size_t i = 0; i < ap.size(); ++i)
		add_blockfield(lb, *ap[i]);
	for (size_t i = 0; i < ep.size(); ++i)
		add_blockfield(rb, *ep[i]);
	for (size_t i = 0; i < np.size(); ++i)
		add_blockfield(rb, *np[i]);
	
	view::clear_screen();

	cout << endl;

	for (int i = 0; i < block_count; ++i){
		bool lhas = false; 
		bool rhas = false;
		if (int(lb.size()) > i) lhas = true;
		if (int(rb.size()) > i) rhas = true;

		if (lhas){
			pad(); cc(lb[i][name][0].first.c_str(), lb[i][name][0].second); next_col(lb[i][name][0].first.length());
		}
		else { pad();  next_col(0); }

		if (rhas){
			pad(); cc(rb[i][name][0].first.c_str(), rb[i][name][0].second);  cout << endl;
		}
		else cout << endl;

		if (lhas){
			print_health_bars(lb[i]);
			next_col(lb[i][health][0].first.length() + lb[i][health][1].first.length() + lb[i][temp_hp][0].first.length() + lb[i][temp_hp][1].first.length() + 4);
		}
		else { pad();  next_col(0); }

		if (rhas){
			print_health_bars(rb[i]);
			cout << endl;
		}
		else cout << endl;

		if (lhas){
			print_turn_bars(lb[i]);
			next_col(lb[i][turn_progress][0].first.length() + lb[i][turn_progress][1].first.length() + 4);
		}
		else { pad();  next_col(0); }

		if (rhas) {
			print_turn_bars(rb[i]);
			cout << endl;
		}
		else cout << endl;

		if (lhas){
			pad(); cc("ST: ", view::magenta);
			print_status_bars(lb[i], 0, 7);
			int skip = 0;
			for (int j = 0; j < 7; ++j){
				if (int(lb[i][stat_eff].size()) > j)
					skip += lb[i][stat_eff][j].first.length();
			}
			next_col(skip + 4);
		}
		else { pad();  next_col(0); }

		if (rhas) {
			pad(); cc("ST: ", view::magenta);
			print_status_bars(rb[i], 0, 7);
			cout << endl;
		}
		else cout << endl;

		if (lhas){
			print_status_bars(lb[i], 7, 14);
			int skip = 0;
			for (int j = 7; j < 14; ++j){
				if (int(lb[i][stat_eff].size())  > j)
					skip += lb[i][stat_eff][j].first.length();
			}
			next_col(skip + 4);
		}
		else { pad();  next_col(0); }

		if (rhas) {
			print_status_bars(rb[i], 7, 14);
			cout << endl;
		}
		else cout << endl;

		cout << endl;
	}
}
void BattleView::print_battle_log(){
	using namespace std;
	using namespace view;
	cout << endl;
	string border = "";
	for (int i = 0; i < total_width; ++i){
		border += "=";
	}
	pad();  ccn(border.c_str(), view::white);
	for (size_t i = battle_log.size() - 20; i < battle_log.size(); ++i){
		pad(); pad(); ccn(battle_log[i]);
	}
	pad(); ccn(border.c_str(), view::white);
}
void BattleView::print_menus() {

}
void BattleView::add_to_battle_log(std::string s, int c) {
	std::vector<std::pair<std::string, int> > v;
	v.push_back(std::make_pair(s, c));
	battle_log.push_back(v);
}
void BattleView::add_to_battle_log(const std::vector<std::pair<std::string, int> > &vp){
	battle_log.push_back(vp);
	return;
}