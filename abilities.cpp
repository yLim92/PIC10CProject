#include "abilities.h"
#include "battlephase.h"
#include "gameunit.h"
#include "utility.h"
#include "status.h"

#include <iostream>
#include <sstream>

Ability::Ability() {

}

Ability::Ability(GameUnit *g){
	set_defaults();

	base_accuracy = 80;

	owner = g;
	name = "Attack";
	initial_desc = get_owner().get_name() + " strikes!";
	delayed_desc = get_owner().get_name() + " strikes!";
}

Ability::~Ability() {

}
void Ability::set_defaults(){
	delay = 0;
	health_effect = true;
	damaging = true;
	status_effect = false;
	target_type = gc::TargetType::single_enemy;

	con_effect_mod = 0;
	str_effect_mod = 0;
	dex_effect_mod = 0;
	int_effect_mod = 0;
	
	crit_damage_mod = 0;
	base_crit_chance = 0;
	base_crit_dmg_mod = 0;
	base_accuracy = 0;

	base_low_bound = 85;
	base_up_bound = 100;

	for (int i = 0; i < 10; ++i)
		status_keys.push_back(utility::base_32_hex());
}

const int Ability::effect_magnitude() const{
	return int(
		(get_owner().get_constitution()*con_effect_mod +
		get_owner().get_strength()*str_effect_mod +
		get_owner().get_dexterity()*dex_effect_mod +
		get_owner().get_intelligence()*int_effect_mod +
		get_owner().get_strength()*0.2)*( 1 + get_owner().get_status_mod(gc::StatusType::ability)));
}
const int Ability::mod_only_magnitude() const{
	return int(
		(get_owner().get_constitution()*con_effect_mod +
		get_owner().get_strength()*str_effect_mod +
		get_owner().get_dexterity()*dex_effect_mod +
		get_owner().get_intelligence()*int_effect_mod)*( 1 + get_owner().get_status_mod(gc::StatusType::ability)));
}
const double Ability::effect_spread(bool perfect) const{
	if (perfect)
		return (1.2 * base_up_bound)/100.;
	else
		return utility::rng(base_low_bound, base_up_bound)/100.;
}
const int Ability::critical_chance() const{
	return base_crit_chance + get_owner().get_critical_chance();
}
const double Ability::critical_damage_multiplier() const{
	return base_crit_dmg_mod + get_owner().get_critical_damage_mod();
}
const int Ability::accuracy() const{
	return base_accuracy + get_owner().get_accuracy();
}
/*
const int Ability::status_effect_chance() const{
	return get_owner().get_effect_accuracy();
}*/
const bool Ability::is_status_successful(int base_acc, double mult, GameUnit &tgt) const{
	int effect_resist = 0;
	if (!get_owner().is_friendly_with(tgt))
		effect_resist = tgt.get_effect_resist();
	return int((base_acc + get_owner().get_effect_accuracy()) * mult) - effect_resist - utility::rng(100) >= 0;
}


int Ability::do_ability_phase(std::vector<GameUnit*> &combatants, BattleView &bv){
	std::vector<GameUnit *> targets;
	if (!target_step(combatants, targets))
		return 0;

	deduct_ability_cost();

	bv.add_to_battle_log(get_initial_description(), view::white); 

	if (get_delay() == 0)
		effect_step(targets, bv);
	else {
		AttributeList attr = AttributeList();
		attr.type = gc::StatusType::channelling;
		attr.duration = get_delay();
		get_owner().add_status(new Status(attr, this, status_keys.back(), get_owner().get_status_list()));
		get_owner().start_channelling();
		get_owner().add_delayed_ability(DelayedAbility(*this, get_delay(), targets));
		apply_statuses(get_owner(), 1);
	}

	return 1;
}
bool Ability::target_step(std::vector<GameUnit*> &combatants, std::vector<GameUnit *> &targets){
	std::vector<GameUnit *> possible_tgts;
	
	set_possible_targets(combatants, possible_tgts);
	
	return select_targets(possible_tgts, targets);
}
void Ability::set_possible_targets(std::vector<GameUnit*> &combatants, std::vector<GameUnit *> &possible_tgts) {
	using namespace gc;
	Affiliation afl = get_owner().get_affiliation();
	for (size_t i = 0; i < combatants.size(); ++i){
		if (combatants[i]->is_defeated())
			continue;
		gc::Affiliation c_afl = combatants[i]->get_affiliation();
		if (get_target_type() == gc::TargetType::self){
			possible_tgts.push_back(&get_owner());
			break;
		}
		else if (get_target_type() == gc::TargetType::single_friendly || get_target_type() == gc::TargetType::party_friendly){
			//If affiliation is player or ally and combatant affiliation is player or ally; or if the affiliations are the same
			if (  ((afl == Affiliation::player || afl == Affiliation::ally) && (c_afl == Affiliation::player || c_afl == Affiliation::ally)) || (afl == c_afl) )
				possible_tgts.push_back(combatants[i]);
		}
		else if (get_target_type() == gc::TargetType::single_enemy) {
			if (  !((afl == Affiliation::player || afl == Affiliation::ally) && (c_afl == Affiliation::player || c_afl == Affiliation::ally)) && !(afl == c_afl) ){
				possible_tgts.push_back(combatants[i]);
			}
		}
	}
}

//Prompts player to select targets based on the tgt array passed in OR
//	has npc do their auto targetting
//Push_backs the targets vector
//Returns true if targets were set; false if back was chosen
bool Ability::select_targets(std::vector<GameUnit*> &poss_tgts, std::vector<GameUnit *> &targets) {
	int sz = poss_tgts.size();
	if (get_owner().get_affiliation() == gc::Affiliation::player) {
		if (is_single_target()){
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
		else {
			if (get_target_type() == gc::TargetType::party_friendly)
				view::ccn(std::to_string(1) + ". All allies" , view::white);
			else
				view::ccn(std::to_string(1) + ". All enemies" , view::white);
			std::cout << 2 << ". Back" << std::endl;
			int user_input = utility::get_user_input(2);
			if (user_input == 1){
				targets = poss_tgts;
				return true;
			}
			else
				return false;
		}
	}
	else{
		get_owner().apply_targetting_logic(poss_tgts, targets, 1, get_target_type());
		return true;
	}
}

void Ability::effect_step(std::vector<GameUnit *> &targets, BattleView &bv){
	int sz = targets.size();
	for (int i = 0; i < sz; ++i) {
		if (targets[i]->is_defeated())
			bv.add_to_battle_log(targets[i]->get_name() + " is already defeated!", view::white);
		else {
			if (!is_friendly()){
				do_harmful_effect(*targets[i], bv, changes_health(), changes_status(), true);
			}
			else {
				double calc_magnitude = effect_magnitude();
				if (changes_health()){
					if (is_damaging())
						calc_magnitude *= -1;
					targets[i]->change_hp(int(calc_magnitude));
					print_health_change(targets[i], int(calc_magnitude), bv);
				}
				if (changes_status()){
					apply_statuses(*targets[i], 1);
				}
			}
		}
	}
}
void Ability::set_pre_hit_values(GameUnit &tgt, bool &hit, double &magn, double &mult, BattleView &bv) {
	int acc = accuracy() - tgt.get_dodge() - utility::rng(100);
	int calc_crit = critical_chance() - utility::rng(100);
	if (acc >= 0){
		hit = true;
		if (acc >= gc::PERFECT_HIT_THRESHOLD){
			magn = effect_magnitude() * effect_spread(true);
			mult *= 2;
			bv.add_to_battle_log("A perfect hit!", view::yellow);
		}
		if (calc_crit >= 0){
			magn *= critical_damage_multiplier();
			mult *= 2;
			bv.add_to_battle_log("CRITICAL!!", view::red);
		}
	}
	if (DEBUG_BATTLE){
		std::cout << "Final Acc: " << acc << std::endl;
		cout << "Final Crit Chance: " << calc_crit << endl;
	}
}
void Ability::do_harmful_effect(GameUnit &tgt, BattleView &bv, bool does_damage, bool does_status, bool do_post_phase){
	bool hit = false;
	double calc_magnitude = effect_magnitude() * effect_spread(false);;
	double calc_effect_chance_mult = 1;
	
	set_pre_hit_values(tgt, hit, calc_magnitude, calc_effect_chance_mult, bv);

	if (hit){
		if (does_damage)
			damage_step(tgt, calc_magnitude, bv);
		if (does_status)
			apply_statuses(tgt, calc_effect_chance_mult);
	}
	else 
		bv.add_to_battle_log(get_owner().get_name() + " misses...", view::white);

	if (do_post_phase)
		post_attack_phase(hit, tgt, int(calc_magnitude), calc_effect_chance_mult, bv);
	if (DEBUG_BATTLE){
		system("pause");
	}
}
void Ability::damage_step(GameUnit &tgt, double &magn, BattleView &bv) {
	double defense_mod = double(tgt.get_damage_reduction() - get_owner().get_armor_ignore())/100.;
	if (defense_mod < 0)
		defense_mod = 0;
	else if (defense_mod > 1)
		defense_mod = 1;
	magn *= -(1 - defense_mod);
		
	tgt.change_hp(int(magn));
	print_health_change(&tgt, int(magn), bv);
	
	if (DEBUG_BATTLE){
		std::cout << "Pre-Armor Magnitude: " << magn/(1 - defense_mod) << std::endl;
		std::cout << "Final Magnitude: " << magn << std::endl;
		std::cout << "Acc: " << accuracy() << std::endl;
		std::cout << "Dodge: " << tgt.get_dodge() << std::endl;
		std::cout << "Total Damage Reduction: " << defense_mod << std::endl;
	}
}
void Ability::post_attack_phase(bool hit, GameUnit &tgt, int magn, double mult, BattleView &bv){
	if (hit){
		if (get_owner().get_status_list().is_affected_by(gc::StatusType::ability_status_on_hit))
			get_owner().get_status_list().do_ability_statuses(gc::StatusType::ability_status_on_hit, tgt, magn, mult, bv, *this, get_owner());
		if (tgt.get_status_list().is_affected_by(gc::StatusType::ability_status_on_dmg))
			tgt.get_status_list().do_ability_statuses(gc::StatusType::ability_status_on_dmg, get_owner(), magn, mult, bv, *this, get_owner());
	}
}
void Ability::print_health_change(GameUnit *tgt, int magn, BattleView &bv) {
	if (magn <= 0){
		std::vector<std::pair<std::string, int> > text;
		text.push_back(std::make_pair(tgt->get_name() + " takes ", view::white)); text.push_back(std::make_pair(std::to_string(-magn), view::red)); text.push_back(std::make_pair(" damage.", view::white));
		bv.add_to_battle_log(text);
	}
	else {
		std::vector<std::pair<std::string, int> > text;
		text.push_back(std::make_pair(tgt->get_name() + " recovers ", view::white)); text.push_back(std::make_pair(std::to_string(magn), view::green)); text.push_back(std::make_pair(" hp.", view::white));
		bv.add_to_battle_log(text);
	}
}
//
////Attempts to remove delayed ability, if it exists in the battlephase's delayed ability list
//void Ability::remove_delayed_ability(const Ability& a) {
//	for(size_t i = 0; i < delayed_abilities.size(); ++i){
//		if (delayed_abilities[i].ability == &a)
//			delayed_abilities.erase(delayed_abilities.begin() + i);
//	}
//}
void Ability::do_delayed_ability(std::vector<GameUnit *> &targets, BattleView &bv) {
	bv.add_to_battle_log(get_delayed_description(), view::white);
	effect_step(targets, bv);
}

void Ability::apply_statuses(GameUnit &tgt, double mult){
	std::vector<AttributeListGroup> algs = status_templates();
	for (int i = 0; i < int(algs.size()); ++i) {
		if (is_status_successful(algs[i].get_main_effect_accuracy(), mult, tgt)){
			std::vector<std::pair<Status*, bool> > statuses;
			AttributeListGroup attr_group = algs[i];
			int sz = attr_group.get_status_count();
			for (int j = 0; j < sz; ++j){
				if (is_status_successful(attr_group.get_effect_chance(j), mult, tgt)){
					if (attr_group.get_target_logic(j))//if self-targetting ability
						statuses.push_back(std::make_pair(
						new Status(attr_group.get_attribute_list(j), this, attr_group.get_id_key(j), get_owner().get_status_list()), 
						attr_group.get_target_logic(j)) );
					else{
						statuses.push_back(std::make_pair(
						new Status(attr_group.get_attribute_list(j), this, attr_group.get_id_key(j), tgt.get_status_list()), 
						attr_group.get_target_logic(j)) );
					}
				}
				else
					statuses.push_back(std::make_pair(nullptr, false));
				
			}
			std::vector<std::pair<int, int> > ll = attr_group.get_link_logic();
			for (int j = 0; j < int(ll.size()); ++j){
				//If the statuses (statuses.first) at index ll.first and ll.second are not nullptrs, link them
				if (statuses[ll[j].first].first != nullptr && statuses[ll[j].second].first != nullptr)
					statuses[ll[j].first].first->link_status(statuses[ll[j].second].first);
			}

			for(int j = 0; j < int(statuses.size()); ++j){
				if (statuses[j].first != nullptr){
					if (statuses[j].second) //If self-targetting ability
						get_owner().add_status(statuses[j].first);
					else
						tgt.add_status(statuses[j].first);
				}
			}
			
		}
	}
}
const std::vector<AttributeListGroup> Ability::status_templates() const{
	std::vector<AttributeListGroup> alg;
	return alg;
}
const void Ability::link_all(vector<pair<int, int> > &ll, int n) const{
	for (int i = 0; i <n; ++i)
		for (int j = i+1; j < n; ++j)
			ll.push_back(std::make_pair(i, j));
}
const bool Ability::is_friendly() const { 
	if (target_type == gc::TargetType::self ||
		target_type == gc::TargetType::single_friendly ||
		target_type == gc::TargetType::party_friendly)
		return true;
	else
		return false;
}
const bool Ability::is_single_target() const{
	if (target_type == gc::TargetType::self ||
		target_type == gc::TargetType::single_friendly ||
		target_type == gc::TargetType::single_enemy)
		return true;
	else
		return false;
}
ChargeStrike::ChargeStrike(GameUnit *g) : Ability(g) {
	set_defaults();

	delay = int(1.5 * gc::FPS);

	str_effect_mod = 2;

	base_accuracy = 90;

	name = "Charge Strike";
	initial_desc = get_owner().get_name() + " gathers energy...";
	delayed_desc = get_owner().get_name() + " unleashes a ferocious blow!";
}
WarriorAbility::WarriorAbility(Soldier *o) : warrior(o) {
	set_defaults();

	base_cd = 10;
	remaining_cd = 0;
}
const GameUnit& WarriorAbility::get_owner() const {
	return *warrior;
}
GameUnit& WarriorAbility::get_owner() {
	return *warrior; 
}
const bool WarriorAbility::is_usable() const {
	if(remaining_cd <= 0)
		return true;
	else
		return false;
}
const std::string WarriorAbility::get_cost_display() const { 
	if(remaining_cd > 0){
		std::stringstream ss;
		double rem = remaining_cd/double(gc::FPS);
		ss << rem;
		return ss.str().substr(0, ss.str().find('.')+3);
	}
	else
		return "";
} 
void WarriorAbility::deduct_ability_cost() {
	remaining_cd = base_cd/warrior->get_resource_mod();
}

MageAbility::MageAbility(Mage* o) {
	set_defaults();

	is_sustaining = false;
	base_mp_cost = 0;
	mage = o;
}

const GameUnit& MageAbility::get_owner() const{
	return *mage;
}
GameUnit& MageAbility::get_owner() {
	return *mage;
}
const bool MageAbility::is_usable() const{
	if(mage->get_mp() >= this->base_mp_cost && !is_sustaining)
		return true;
	else
		return false;
}
const std::string MageAbility::get_cost_display() const{
	return std::to_string(base_mp_cost);
}
void MageAbility::deduct_ability_cost() {
	mage->change_mp(-base_mp_cost);
}
void MageAbility::start_sustaining() { 
	is_sustaining = true; 
	mage->add_sustained_ability(this);
}
void MageAbility::stop_sustaining() { 
	is_sustaining = false;
	get_owner().get_status_list().remove_status_and_links(sustain_status.first, sustain_status.second);
}

RogueAbility::RogueAbility(Thief* o) {
	set_defaults();

	combo_point_change[gc::ComboPointType::attack]	= 0;
	combo_point_change[gc::ComboPointType::dodge]	= 0;
	combo_point_change[gc::ComboPointType::disrupt]	= 0;

	rogue = o;
}

const GameUnit& RogueAbility::get_owner() const{
	return *rogue;
}
GameUnit& RogueAbility::get_owner() {
	return *rogue;
}
const bool RogueAbility::is_usable() const{
	if(rogue->get_combo_points() < -combo_point_change)
		return false;
	else
		return true;
}
const std::string RogueAbility::get_cost_display() const{
	std::string s = "";
	for (auto it = combo_point_change.begin(); it != combo_point_change.end(); ++it)
		s += std::to_string(it->second) + " ";
	return s;
}
void RogueAbility::deduct_ability_cost() {
	rogue->change_combo_points(combo_point_change);
}

MoraleBoost::MoraleBoost(Soldier *o) : WarriorAbility(o) {
	damaging =		false;
	status_effect = true;
	target_type = gc::TargetType::single_friendly;

	int_effect_mod = 1.0;
	base_cd = 10.0 * gc::FPS;

	name = "Morale Boost";
	initial_desc = warrior->get_name() + " rallies an ally!";
}
const std::vector<AttributeListGroup> MoraleBoost::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::ability;
	attr.duration = 10 * gc::FPS;
	attr.effect_magnitude = 0.5;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	v_algs.push_back(alg);
	return v_algs;
}

Hold::Hold(Soldier* o) : WarriorAbility(o) {
	health_effect = false;
	status_effect = true;

	base_accuracy = 100;

	base_cd = 20.0 * gc::FPS;

	name = "Restrain";
	initial_desc = warrior->get_name() + " attempts to grab the target...";

}
const std::vector<AttributeListGroup> Hold::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::channelling;
	attr.duration = 10 * gc::FPS;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	attr = AttributeList();
	attr.type = gc::StatusType::grab;
	attr.duration = 1 * gc::FPS;
	attr.sustain_chance = 0.8;
	attr.sustain_chance_decay = 0.05;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);
	/*
	attr = AttributeList();
	attr.type = gc::StatusType::poison;
	attr.duration = gc::PRACTICALLY_INFINITY;
	attr.effect_magnitude = 50;
	alg.add_attribute_list(attr, false, 50, status_keys[key_index++]);*/

	std::vector<std::pair<int, int >> ll;
	ll.push_back(std::make_pair(0, 1));
	alg.set_link_logic(ll);

	v_algs.push_back(alg);

	return v_algs;
}

Provoke::Provoke(Soldier* o) : WarriorAbility(o) {
	status_effect = true;

	con_effect_mod = 0.2;
	str_effect_mod = 0.6;
	
	base_accuracy = 90;

	base_cd = 15.0 * gc::FPS;

	name = "Provoke";
	initial_desc = warrior->get_name() + " does an attention grabbing attack!";
}
const std::vector<AttributeListGroup> Provoke::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::challenged;
	attr.duration = 20 * gc::FPS;
	attr.effect_magnitude = int(gc::Threat::low);
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	v_algs.push_back(alg);
	return v_algs;
}

Retaliate::Retaliate(Soldier* o) : WarriorAbility(o) {
	health_effect = false;
	status_effect = true;
	target_type = gc::TargetType::self;

	con_effect_mod = 0.5;
	str_effect_mod = 1.0;

	base_cd = 30.0 * gc::FPS;
	base_accuracy = 100;

	name = "Retaliate";
	initial_desc = warrior->get_name() + " prepares to retaliate...";
	delayed_desc = warrior->get_name() + " counterattacks!";
}
const std::vector<AttributeListGroup> Retaliate::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::ability_status_on_dmg;
	attr.duration = 15 *gc::FPS;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	v_algs.push_back(alg);
	return v_algs;
}
void Retaliate::do_ability_as_status(gc::StatusType, GameUnit &tgt, int magn, double mult, BattleView &bv, Ability &trig_abl, GameUnit &current) {
	bv.add_to_battle_log(get_delayed_description(), view::white);

	do_harmful_effect(tgt, bv, true, false, false);
}
Batter::Batter(Soldier* o) : WarriorAbility(o) {
	status_effect = true;

	str_effect_mod = 1.5;
	base_accuracy = 80;

	base_cd = 30.0 * gc::FPS;
	
	consecutive_uses = 0;
	free_usage_timer = 0;

	name = "Batter";
	initial_desc = warrior->get_name() + " strikes relentlessly!";
}
const std::vector<AttributeListGroup> Batter::status_templates() const {
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::speed;
	attr.duration = 5 *gc::FPS;
	attr.effect_magnitude = 0.5 * (2 + consecutive_uses);
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	v_algs.push_back(alg);
	return v_algs;
}
void Batter::decrement_cd() {
	if (free_usage_timer > 0)
		free_usage_timer--;
	else if (free_usage_timer == 0 && consecutive_uses > 0)
		consecutive_uses = 0;
	else 
		remaining_cd--;
}
const bool Batter::is_usable() const{
	if(remaining_cd <= 0 || free_usage_timer > 0)
		return true;
	else
		return false;
}
void Batter::post_attack_phase(bool hit, GameUnit &tgt, int magn, double mult, BattleView &bv) {
	if (hit){
		free_usage_timer = 5*gc::FPS;
		++consecutive_uses;
	}
	else {
		free_usage_timer = 0;
		consecutive_uses = 0;
	}
	Ability::post_attack_phase(hit, tgt, magn, mult, bv);
}
const int Batter::accuracy() const{
	return base_accuracy + get_owner().get_accuracy() - 5 *consecutive_uses;
}
Cleave::Cleave(Soldier* o) : WarriorAbility(o) {
	str_effect_mod = 0.7;
	dex_effect_mod = 0.3;
	base_accuracy = 70;

	base_cd = 15 * gc::FPS;

	name = "Cleave";
	initial_desc = warrior->get_name() + " swings widely!";
}
bool Cleave::select_targets(std::vector<GameUnit *> &poss_tgts, std::vector<GameUnit *> &targets) {
	bool tgt_selected = Ability::select_targets(poss_tgts, targets);
	
	if (tgt_selected){
		int extra_targets = utility::rng(0, 2);
		extra_targets = min( extra_targets, int(poss_tgts.size() - 1));

		std::vector<int> poss_indexes;
		for (size_t i = 0; i < poss_tgts.size(); ++i)
			poss_indexes.push_back(i);
		while(extra_targets > 0){
			int index = utility::rng(0, int(poss_indexes.size()-1));
			if (std::find(targets.begin(), targets.end(), poss_tgts[poss_indexes[index]]) == targets.end()){
				targets.push_back(poss_tgts[poss_indexes[index]]);
				--extra_targets;
			}
			poss_indexes.erase(poss_indexes.begin() + index);
		}
	}
	return tgt_selected;
}

Resolve::Resolve(Soldier* o) : WarriorAbility(o) {
	health_effect = false;
	damaging = false;
	status_effect = true;
	target_type = gc::TargetType::party_friendly;

	int_effect_mod = 0.1;
	str_effect_mod = 0.05;

	base_cd = 60 * gc::FPS;

	name = "Resolve";
	initial_desc = warrior->get_name() + " orders the party to stand firm!";
}
const std::vector<AttributeListGroup> Resolve::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	int key_index = 0;


	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::effect_resist;
	attr.duration = 20 * gc::FPS;
	attr.effect_magnitude = (15 + mod_only_magnitude())/100.;
	attr.effect_decay = 0.05;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	attr = AttributeList();
	attr.type = gc::StatusType::damage_reduction;
	attr.duration = 20 * gc::FPS;
	attr.effect_magnitude = (15 + mod_only_magnitude())/100.;
	attr.effect_decay = 0.05;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	v_algs.push_back(alg);
	return v_algs;
}
Strike::Strike(Soldier* o) : WarriorAbility(o) {
	str_effect_mod = 1.0;

	base_accuracy = 90;

	base_cd = 0 * gc::FPS;

	name = "Strike";
	initial_desc = warrior->get_name() + " strikes!";
}

Fireball::Fireball(Mage *o) : MageAbility(o) {
	status_effect = true;

	int_effect_mod = 1.2;

	base_accuracy = 95;

	base_mp_cost = 10;

	name = "Fireball";
	initial_desc = get_owner().get_name() + " hurls a fireball!";

}
const std::vector<AttributeListGroup> Fireball::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(20);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::burn;
	attr.duration = 1 * gc::FPS;
	attr.effect_magnitude = effect_magnitude()*0.5;
	attr.effect_decay = 0.1;
	attr.sustain_chance = 1.0;
	attr.sustain_chance_decay = 0.2;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	v_algs.push_back(alg);
	return v_algs;
}

Meteor::Meteor(Mage *o) : MageAbility(o) {
	delay = 4 * gc::FPS;

	int_effect_mod = 4;

	base_accuracy = 80;
	base_low_bound = 50;

	base_mp_cost = 50;

	name = "Meteor";
	initial_desc = get_owner().get_name() + " begins channelling...";
	delayed_desc = "A meteor crashes onto the battlefield!";
}

Bolt::Bolt(Mage *o) : MageAbility(o) {
	status_effect = true;

	int_effect_mod = 1.5;

	base_accuracy = 95;

	base_mp_cost = 30;

	name = "Bolt";
	initial_desc = get_owner().get_name() + " fires a bolt of lightning!";

}
const std::vector<AttributeListGroup> Bolt::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(30);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::stun;
	attr.duration = 3 * gc::FPS;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	v_algs.push_back(alg);
	return v_algs;
}

LightningShield::LightningShield(Mage *o) : MageAbility(o) {
	health_effect = false;
	status_effect = true;
	target_type = gc::TargetType::single_friendly;

	int_effect_mod = 1.0;
	base_accuracy = 100;

	base_mp_cost = 10;

	name = "Lightning Array";
	initial_desc = get_owner().get_name() + " creates a matrix of lightning around an ally!";
	delayed_desc = "The lightning matrix distorts!";

}

//Need to find a better way around hardcoding numbers for the status_keys
const std::vector<AttributeListGroup> LightningShield::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::ability_status_on_hit;
	attr.duration = 15 * gc::FPS;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[0]);

	attr = AttributeList();
	attr.type = gc::StatusType::ability_status_on_dmg;
	attr.duration = 15 * gc::FPS;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[1]);

	std::vector<std::pair<int, int >> ll;
	ll.push_back(std::make_pair(0, 1));
	alg.set_link_logic(ll);

	v_algs.push_back(alg);
	return v_algs;
}
void LightningShield::do_ability_as_status(gc::StatusType st, GameUnit &tgt, int magn, double mult, BattleView &bv, Ability &trig_abl, GameUnit &current){
	if (st == gc::StatusType::ability_status_on_hit){
		AttributeList attr = AttributeList();
		attr.type = gc::StatusType::stun;
		attr.duration = 3 * gc::FPS;
		Status *s = new Status(attr, this, status_keys[2], tgt.get_status_list());

		if (trig_abl.is_status_successful(30, mult, tgt) ){
			bv.add_to_battle_log(current.get_name() + " stuns the enemy!", view::white);
			tgt.add_status(s);
		}
	}
	else if (st == gc::StatusType::ability_status_on_dmg){
		bv.add_to_battle_log(get_delayed_description(), view::white);
		do_harmful_effect(tgt, bv, true, false, false);
	}
}
Barrier::Barrier(Mage *o) : MageAbility(o) {
	health_effect = false;
	status_effect = true;
	target_type = gc::TargetType::single_friendly;

	con_effect_mod = 1.5;
	int_effect_mod = 2.5;

	base_mp_cost = 10;

	name = "Barrier";
	initial_desc = get_owner().get_name() + " shields an ally!";
}
const std::vector<AttributeListGroup> Barrier::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::temp_hp_up;
	attr.duration = 10 * gc::FPS;
	attr.effect_magnitude = effect_magnitude();
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[++key_index]);

	v_algs.push_back(alg);
	return v_algs;
}
void Barrier::apply_statuses(GameUnit &tgt, double mult){
	Ability::apply_statuses(tgt, mult);
	tgt.change_temp_hp(effect_magnitude());
}
Enfeeble::Enfeeble(Mage* o) : MageAbility(o) {
	health_effect = false;
	status_effect = true;
	target_type = gc::TargetType::single_enemy;

	int_effect_mod = 2.5;

	base_accuracy = 100;

	base_mp_cost = 10;

	name = "Enfeeble";
	initial_desc = get_owner().get_name() + " channels a weakening curse!";
	sustain_status = std::make_pair(gc::StatusType::channelling, status_keys[0]);
}
const std::vector<AttributeListGroup> Enfeeble::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::channelling;
	attr.duration = 10 * gc::FPS;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	attr = AttributeList();
	attr.type = gc::StatusType::threat_level;
	attr.effect_magnitude = int(gc::Threat::mid);
	attr.duration = gc::PRACTICALLY_INFINITY;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	attr = AttributeList();
	attr.type = gc::StatusType::regen_resource;
	attr.effect_magnitude = -2.0;
	attr.duration = gc::PRACTICALLY_INFINITY;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	attr = AttributeList();
	attr.type = gc::StatusType::dodge;
	attr.duration = gc::PRACTICALLY_INFINITY;
	attr.effect_magnitude = -0.1;
	attr.effect_decay = - 1;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);
	
	attr = AttributeList();
	attr.type = gc::StatusType::damage_reduction;
	attr.duration = gc::PRACTICALLY_INFINITY;
	attr.effect_magnitude = -0.1;
	attr.effect_decay = - 1;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	attr = AttributeList();
	attr.type = gc::StatusType::speed;
	attr.duration = gc::PRACTICALLY_INFINITY;
	attr.effect_magnitude = -0.1;
	attr.effect_decay = - 2;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	
	std::vector<std::pair<int, int >> ll;
	link_all(ll, key_index);
	alg.set_link_logic(ll);

	v_algs.push_back(alg);

	return v_algs;
}
void Enfeeble::post_attack_phase(bool hit, GameUnit &tgt, int magn, double mult, BattleView &bv) {
	Ability::post_attack_phase(hit, tgt, magn, mult, bv);
	if (hit)
		start_sustaining();
}

Curse::Curse(Mage* o) : MageAbility(o) {
	health_effect = false;
	status_effect = true;
	target_type = gc::TargetType::single_enemy;

	int_effect_mod = 0.5;

	base_accuracy = 100;

	base_mp_cost = 10;

	name = "Defile";
	initial_desc = get_owner().get_name() + " poisons the enemy with shadow magic!";
	sustain_status = std::make_pair(gc::StatusType::regen_resource, status_keys[0]);
}

const std::vector<AttributeListGroup> Curse::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);
	int key_index = 0;

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::regen_resource;
	attr.effect_magnitude = -0.5;
	attr.duration = gc::PRACTICALLY_INFINITY;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	attr = AttributeList();
	attr.type = gc::StatusType::poison;
	attr.effect_magnitude = effect_magnitude();
	attr.duration = gc::PRACTICALLY_INFINITY;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY, status_keys[key_index++]);

	std::vector<std::pair<int, int >> ll;
	ll.push_back(std::make_pair(0,1));
	alg.set_link_logic(ll);

	v_algs.push_back(alg);

	return v_algs;
}

void Curse::post_attack_phase(bool hit, GameUnit &tgt, int magn, double mult, BattleView &bv) {
	Ability::post_attack_phase(hit, tgt, magn, mult, bv);
	if (hit)
		start_sustaining();
}

RogueAttack::RogueAttack(Thief* o): RogueAbility(o) {
	dex_effect_mod = 0.5;

	base_accuracy = 100;

	combo_point_change[gc::ComboPointType::attack]	= 1;
	combo_point_change[gc::ComboPointType::dodge]	= 0;
	combo_point_change[gc::ComboPointType::disrupt]	= 0;

	name = "Attack";
	initial_desc = get_owner().get_name() + " takes a quick stab at the enemy!";
}

Snipe::Snipe(Thief* o): RogueAbility(o) {
	dex_effect_mod = 2.0;

	base_accuracy = 150;
	base_crit_chance = 20;

	combo_point_change[gc::ComboPointType::attack]	= -3;
	combo_point_change[gc::ComboPointType::dodge]	= 0;
	combo_point_change[gc::ComboPointType::disrupt]	= 0;

	name = "Snipe";
	initial_desc = get_owner().get_name() + " takes a careful shot at the enemy...";
}
