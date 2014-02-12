#include<iostream>
#include<exception>
#include <iomanip> 

#include "battlephase.h"
#include "gameunit.h"
#include "abilities.h"

#include "utility.h"

#define DEBUG_BATTLE 1

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
		gu_turn[player_party->get_party_members()[i]] = new PlayerTurn(this, player_party->get_party_members()[i]);
		combatants.push_back(player_party->get_party_members()[i]);
	}
	for (size_t i = 0; i < enemy_party->get_party_members().size(); i++){
		gu_turn[enemy_party->get_party_members()[i]] = new NpcTurn(this, enemy_party->get_party_members()[i]);
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
		gu_turn[player_party->get_party_members()[i]] = new PlayerTurn(this, player_party->get_party_members()[i]);
		combatants.push_back(player_party->get_party_members()[i]);
	}
	for (size_t i = 0; i < enemy_party->get_party_members().size(); i++){
		gu_turn[enemy_party->get_party_members()[i]] = new NpcTurn(this, enemy_party->get_party_members()[i]);
		combatants.push_back(enemy_party->get_party_members()[i]);
	}
	for (size_t i = 0; i < ally_party->get_party_members().size(); i++){
		gu_turn[ally_party->get_party_members()[i]] = new NpcTurn(this, ally_party->get_party_members()[i]);
		combatants.push_back(ally_party->get_party_members()[i]);
	}
	for (size_t i = 0; i < neutral_party->get_party_members().size(); i++){
		gu_turn[neutral_party->get_party_members()[i]] = new NpcTurn(this, neutral_party->get_party_members()[i]);
		combatants.push_back(neutral_party->get_party_members()[i]);
	}


	bv = new BattleView(this);
}

int BattlePhase::do_battle() {
	while (!is_battle_complete()){
		bv->print_battle_view();
		clock_t begin = clock();
		while ((((double)(clock() - begin)) / CLOCKS_PER_SEC) < 1.0/gc::FPS){}
		for (size_t i = 0; i < delayed_abilities.size(); ++i){
			if (--delayed_abilities[i].delay <= 0){
				do_delayed_ability(*delayed_abilities[i].ability, delayed_abilities[i].targets);
				delayed_abilities.erase(delayed_abilities.begin() + i);
			}
		}

		for (auto i = gu_turn.begin(); i != gu_turn.end(); ++i){
			if (!i->first->is_defeated() && i->first->is_turn()){
				i->second->do_turn();
				//i->second->reset_increment();
			}
		}
		for (size_t i = 0; i < combatants.size(); ++i){
			combatants[i]->update();
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
int BattlePhase::do_ability_phase(Ability &a){
	std::vector<GameUnit *> targets;
	if (!target_step(a, targets))
		return 0;

	a.deduct_ability_cost();

	bv->add_to_battle_log(a.get_initial_description(), view::white); 

	if (a.get_delay() == 0)
		effect_step(a, targets);
	else
		add_delayed_ability(DelayedAbility(&a, a.get_delay(), targets));

	return 1;
}
bool BattlePhase::target_step(const Ability &a, std::vector<GameUnit *> &targets){
	std::vector<GameUnit *> possible_tgts;
	
	set_possible_targets(a,  possible_tgts);
	
	return select_targets(a, possible_tgts, targets);
}
void BattlePhase::set_possible_targets(const Ability &a, std::vector<GameUnit *> &possible_tgts) {
	using namespace gc;
	Affiliation afl = a.get_owner().get_affiliation();
	if (a.is_friendly()){
		if (afl == Affiliation::player ||
			afl == Affiliation::ally){
			for (size_t i = 0; i < player_party->get_party_members().size(); ++i){
				possible_tgts.push_back(player_party->get_party_members()[i]);
			}
			for (size_t i = 0; i < ally_party->get_party_members().size(); ++i){
				possible_tgts.push_back(ally_party->get_party_members()[i]);
			}
		}
		if (afl == Affiliation::enemy){
			for (size_t i = 0; i < enemy_party->get_party_members().size(); ++i){
				possible_tgts.push_back(enemy_party->get_party_members()[i]);
			}
		}
		if (afl == Affiliation::neutral){
			for (size_t i = 0; i < neutral_party->get_party_members().size(); ++i){
				possible_tgts.push_back(neutral_party->get_party_members()[i]);
			}
		}
	}
	else {
		if (afl == Affiliation::player ||
			afl == Affiliation::ally ||
			afl == Affiliation::neutral){
			for (size_t i = 0; i < enemy_party->get_party_members().size(); ++i){
				possible_tgts.push_back(enemy_party->get_party_members()[i]);
			}
		}
		if (afl == Affiliation::enemy ||
			afl == Affiliation::neutral){
			for (size_t i = 0; i < player_party->get_party_members().size(); ++i){
				possible_tgts.push_back(player_party->get_party_members()[i]);
			}
			for (size_t i = 0; i < ally_party->get_party_members().size(); ++i){
				possible_tgts.push_back(ally_party->get_party_members()[i]);
			}
		}
		if (afl == Affiliation::player ||
			afl == Affiliation::ally ||
			afl == Affiliation::enemy){
			for (size_t i = 0; i < neutral_party->get_party_members().size(); ++i){
				possible_tgts.push_back(neutral_party->get_party_members()[i]);
			}
		}
	}
}

//Prompts player to select targets based on the tgt array passed in OR
//	has npc do their auto targetting
//Push_backs the targets vector
//Returns true if targets were set; false if back was chosen
bool BattlePhase::select_targets(const Ability &a, const std::vector<GameUnit*> &poss_tgts, std::vector<GameUnit *> &targets) {
	int sz = poss_tgts.size();
	if (a.get_owner().get_affiliation() == gc::Affiliation::player) {
		std::vector<int> ignore;
		for (int i = 0; i < sz; ++i) {
			if (!poss_tgts[i]->is_defeated())
				view::ccn(std::to_string(i + 1) + ". " + poss_tgts[i]->get_name(), view::white);
			else {
				view::ccn(std::to_string(i + 1) + ". " + poss_tgts[i]->get_name(), view::gray);
				ignore.push_back(i + 1);
			}
		}
		std::cout << sz + 1 << ". Back" << std::endl;
		int user_input = utility::get_user_input(sz + 1, ignore);

		if (user_input <= sz){
			targets.push_back(poss_tgts[user_input - 1]);
			return true;
		}
		else
			return false;
	}
	else{
		targets.push_back(poss_tgts[utility::rng(sz) - 1]);
		return true;
	}
}


void BattlePhase::effect_step(Ability &a, std::vector<GameUnit *> &targets){
	int sz = targets.size();
	for (int i = 0; i < sz; ++i) {
		if (targets[i]->is_defeated())
			bv->add_to_battle_log(targets[i]->get_name() + " is already defeated!", view::white);
		else {
			if (!a.is_friendly()){
				int final_accuracy = a.accuracy() - targets[i]->get_dodge() - utility::rng(100);
				if (final_accuracy >= 0){
					double calc_magnitude;
					double calc_effect_chance_mult = 1;
					if (a.changes_health() && final_accuracy >= gc::PERFECT_HIT_THRESHOLD){
						bv->add_to_battle_log("A perfect hit!", view::yellow);
						calc_magnitude = a.effect_magnitude() * a.effect_spread(true);
						calc_effect_chance_mult *= 2.;
					}
					else {
						calc_magnitude = a.effect_magnitude() * a.effect_spread(false);
					}

					int calc_crit = a.critical_chance() - utility::rng(100);
					if (a.changes_health() && calc_crit >= 0){
						bv->add_to_battle_log("CRITICAL!!", view::red);
						calc_magnitude *= a.critical_damage_multiplier();
						calc_effect_chance_mult *= a.critical_damage_multiplier();
					}

					double defense_mod = double(targets[i]->get_damage_reduction() - a.get_owner().get_armor_ignore())/100.;
					if (defense_mod < 0)
						defense_mod = 0;
					if (a.changes_health()){
						if (a.is_damaging()){
							calc_magnitude /= (1 + defense_mod);
							calc_magnitude *= -1;
						}
						targets[i]->change_hp(int(calc_magnitude));
						print_health_change(targets[i], int(calc_magnitude));
					}

					if (a.changes_status()){
						std::vector<StatusBlock> abl_stats = a.ability_statuses();
						for (size_t j = 0; j < abl_stats.size(); ++j){
							std::cout << "Effect Chance: " << (abl_stats[j].effect_chance + a.status_effect_chance()) * calc_effect_chance_mult - targets[i]->get_effect_resist() << std::endl;
							if ((abl_stats[j].effect_chance + a.status_effect_chance()) * calc_effect_chance_mult - targets[i]->get_effect_resist() - utility::rng(100) >= 0){
								apply_status(a, abl_stats[j], targets[i]);
							}
						}
					}
					if (DEBUG_BATTLE){
						std::cout << "Final Acc: " << final_accuracy << std::endl;
						std::cout << "Pre-Armor Magnitude: " << calc_magnitude*(1 + defense_mod) << std::endl;
						std::cout << "Final Magnitude: " << calc_magnitude << std::endl;
						std::cout << "Acc: " << a.accuracy() << std::endl;
						std::cout << "Dodge: " << targets[i]->get_dodge() << std::endl;
						std::cout << "Damage_reduction: " << defense_mod << std::endl;
						system("pause");
					}
				}
				else {
					bv->add_to_battle_log(a.get_owner().get_name() + " misses...", view::white);
					if (DEBUG_BATTLE){
						std::cout << "Final Acc: " << final_accuracy << std::endl;
						system("pause");
					}
				}
			}
			else {
				double calc_magnitude = a.effect_magnitude();
				if (a.changes_health()){
					if (a.is_damaging())
						calc_magnitude *= -1;
					targets[i]->change_hp(int(calc_magnitude));
					print_health_change(targets[i], int(calc_magnitude));
				}
				if (a.changes_status()){
					std::vector<StatusBlock> abl_stats = a.ability_statuses();
					for (size_t j = 0; j < abl_stats.size(); ++j){
						if (abl_stats[j].effect_chance + a.status_effect_chance() - utility::rng(100) >= 0){
							apply_status(a, abl_stats[j], targets[i]);
						}
					}
				}
			}
		}
	}
}
void BattlePhase::apply_status(Ability &a, StatusBlock sb, GameUnit *tgt) {
	Status *stat = new Status(sb.attr_list, &a);
	std::vector<Status*> linked_stats;
	for (size_t j = 0; j < sb.l_attr_list.size(); ++j){
		Status *l_stat = new Status(sb.l_attr_list[j], &a);
		stat->link_status(l_stat);
		for (size_t k = 0; k < linked_stats.size(); ++k){
			l_stat->link_status(linked_stats[k]);
		}
		linked_stats.push_back(l_stat);
		
	}
	for (size_t j = 0; j < linked_stats.size(); ++j){
		if (sb.linked_self_target)
			a.get_owner().add_status(linked_stats[j]);
		else
			tgt->add_status(linked_stats[j]);
	}
	if (sb.main_self_target)
		a.get_owner().add_status(stat);
	else
		tgt->add_status(stat);
}
void BattlePhase::print_health_change(GameUnit *tgt, int magn) {
	if (magn <= 0){
		std::vector<std::pair<std::string, int> > text;
		text.push_back(std::make_pair(tgt->get_name() + " takes ", view::white)); text.push_back(std::make_pair(std::to_string(-magn), view::red)); text.push_back(std::make_pair(" damage.", view::white));
		bv->add_to_battle_log(text);
	}
	else {
		std::vector<std::pair<std::string, int> > text;
		text.push_back(std::make_pair(tgt->get_name() + " recovers ", view::white)); text.push_back(std::make_pair(std::to_string(magn), view::green)); text.push_back(std::make_pair(" temporary hp.", view::white));
		bv->add_to_battle_log(text);
	}
}

void BattlePhase::add_delayed_ability(DelayedAbility d) {
	delayed_abilities.push_back(d);
}
void BattlePhase::do_delayed_ability(Ability &a, std::vector<GameUnit *> &targets) {
	bv->add_to_battle_log(a.get_delayed_description(), view::white);
	effect_step(a, targets);
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
void BattleView::add_blockfield(std::vector<BattleView::blockfield> &bf_v, const GameUnit &gu, const Turn &tu) {
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
		if (gu.get_hp() * 20 / gu.get_max_hp() > i) hlth += "X";
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
		view::Colors grn = view::green;
		view::Colors red = view::red;
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
			it->first == StatusType::regen_temp_hp ||
			it->first == StatusType::regen_resource){
			if (it->second >= 0) bf[stat_eff].push_back(make_pair(status_shorthand(it->first), grn));
			else bf[stat_eff].push_back(make_pair(status_shorthand(it->first), red));
		}
		else if (it->first == gc::StatusType::channelling ||
			it->first == gc::StatusType::stun ||
			it->first == gc::StatusType::burn ||
			it->first == gc::StatusType::bleed ||
			it->first == gc::StatusType::poison){
			bf[stat_eff].push_back(make_pair(status_shorthand(it->first), red));
		}
		else if (it->first == gc::StatusType::counter)
			bf[stat_eff].push_back(make_pair(status_shorthand(it->first), grn));
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
	else if (st == gc::StatusType::burn)			return "Brn ";
	else if (st == gc::StatusType::bleed)			return "Bld ";
	else if (st == gc::StatusType::poison)			return "Psn ";
	else if (st == gc::StatusType::regen_temp_hp)	return "RTH ";
	else if (st == gc::StatusType::regen_resource)	return "ReR ";
	else if (st == gc::StatusType::counter)			return "Ctr ";
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
		add_blockfield(lb, *pp[i], *(bp->gu_turn[pp[i]]));
	for (size_t i = 0; i < ap.size(); ++i)
		add_blockfield(lb, *ap[i], *(bp->gu_turn[ap[i]]));
	for (size_t i = 0; i < ep.size(); ++i)
		add_blockfield(rb, *ep[i], *(bp->gu_turn[ep[i]]));
	for (size_t i = 0; i < np.size(); ++i)
		add_blockfield(rb, *np[i], *(bp->gu_turn[np[i]]));
	
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


Turn::Turn(BattlePhase *b, GameUnit *g) : bp(b), gu(g)  {
}

int Turn::do_turn() {
	return 0;
}

int PlayerTurn::do_turn() {
	std::cout << gu->get_name() << " Resource: " << gu->get_resource_display() << std::endl;	
	std::cout << "1. Abilities" << std::endl;
	
	int user_input = utility::get_user_input(1);
	if (user_input == 1){
		list_abilities();
	}
	return 0;
}

void PlayerTurn::list_abilities() {
	//std::vector<Ability*> abl = gu->get_abilities();
	std::vector<int> ignore;

	int i = 0;
	while (gu->get_ability(i) != nullptr){
		view::Colors col = view::white;
		if (!gu->get_ability(i)->is_usable()){	
			col = view::gray;
			ignore.push_back(i+1);
		}
		view::cc(std::to_string(i+1) + ": " + gu->get_ability(i)->get_name(), col);
		std::cout << std::setw(25-gu->get_ability(i)->get_name().length()); 
		view::ccn(gu->get_ability(i)->get_cost_display(), col);
		++i;
	}
	int sz = i;
	view::ccn(std::to_string(sz+1) + ": Back", view::white);
	int user_input = utility::get_user_input(sz + 1, ignore);
	if (user_input <= sz){ 
		if (bp->do_ability_phase( *gu->get_ability(user_input - 1) ) ) {
			gu->reset_increment();
			return;
		}
		else //Continue to list abilities if back was selected
			list_abilities();
	}
	else
		do_turn();
	return;
}

int NpcTurn::do_turn() {
	bp->do_ability_phase( *gu->get_ability(utility::rng(gu->get_abilities().size()) - 1) );
	gu->reset_increment();
	return 0;
}