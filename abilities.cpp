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

	owner = g;
	name = "Attack";
	initial_desc = get_owner().get_name() + " strikes!";
	delayed_desc = get_owner().get_name() + " strikes!";
}

Ability::~Ability() {

}
void Ability::set_defaults(){
	delay = 0;
	friendly = false;
	health_effect = true;
	damaging = true;
	status_effect = false;

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
}

const int Ability::effect_magnitude() const{
	return int(
		(get_owner().get_constitution()*con_effect_mod +
		get_owner().get_strength()*str_effect_mod +
		get_owner().get_dexterity()*dex_effect_mod +
		get_owner().get_intelligence()*int_effect_mod +
		get_owner().get_strength()*0.2)*( 1 + get_owner().get_status_mod(gc::StatusType::ability)));
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
const int Ability::status_effect_chance() const{
	return get_owner().get_effect_accuracy();
}

const std::vector<StatusBlock> Ability::ability_statuses() const {
	return statuses;
}
/*
const AttributeList Ability::linked_status_attr() const {
	return AttributeList();
}*/
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
	if(mage->get_mp() >= this->base_mp_cost)
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
	friendly =		true;
	damaging =		false;
	status_effect = true;

	int_effect_mod = 0.5;


	base_cd = 10.0 * gc::FPS;

	name = "Morale Boost";
	initial_desc = warrior->get_name() + " rallies an ally!";
	
	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::ability;
	attr.duration = 10 * gc::FPS;
	attr.effect_magnitude = 0.5;

	StatusBlock sb = StatusBlock();
	sb.attr_list = attr;
	sb.main_self_target = false;
	sb.effect_chance = gc::PRACTICALLY_INFINITY;

	statuses.push_back(sb);
}


Hold::Hold(Soldier* o) : WarriorAbility(o) {
	health_effect = false;
	status_effect = true;

	base_accuracy = 190;

	base_cd = 4.0 * gc::FPS;

	name = "Restrain";
	initial_desc = warrior->get_name() + " attempts to grab the target...";

	StatusBlock sb = StatusBlock();
	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::stun;
	attr.duration = 10 * gc::FPS;
	sb.attr_list = attr;

	AttributeList l_attr = AttributeList();
	l_attr.type = gc::StatusType::stun;
	l_attr.duration = 1 * gc::FPS;
	l_attr.sustain_chance = 1.0;
	//l_attr.duration = 1 * gc::FPS;
	//attr.sustain_chance = 0.8;
	//attr.sustain_chance_decay = 0.05;
	sb.l_attr_list.push_back(l_attr);

	l_attr.type = gc::StatusType::poison;
	l_attr.duration = gc::PRACTICALLY_INFINITY;
	l_attr.effect_magnitude = 75;
	sb.l_attr_list.push_back(l_attr);

	sb.main_self_target = true;
	sb.linked_self_target = false;
	sb.effect_chance = gc::PRACTICALLY_INFINITY;
	
	statuses.push_back(sb);
}

Fireball::Fireball(Mage *o) : MageAbility(o) {
	status_effect = true;

	int_effect_mod = 1.2;

	base_accuracy = 95;

	base_mp_cost = 10;

	name = "Fireball";
	initial_desc = get_owner().get_name() + " hurls a fireball!";

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::burn;
	attr.duration = 1 * gc::FPS;
	attr.effect_magnitude = effect_magnitude()*0.5;
	attr.effect_decay = 0.1;
	attr.sustain_chance = 1.0;
	attr.sustain_chance_decay = 0.2;

	StatusBlock sb = StatusBlock();
	sb.attr_list = attr;
	sb.main_self_target = false;
	sb.effect_chance = 20;

	statuses.push_back(sb);
}

Meteor::Meteor(Mage *o) : MageAbility(o) {
	delay = 4 * gc::FPS;

	int_effect_mod = 4;

	base_accuracy = 80;
	base_low_bound = 40;

	base_mp_cost = 50;

	name = "Meteor";
	initial_desc = get_owner().get_name() + " begins channelling...";
	delayed_desc = "A meteor crashes onto the battlefield!";
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

	base_accuracy = 130;
	base_crit_chance = 20;

	combo_point_change[gc::ComboPointType::attack]	= -3;
	combo_point_change[gc::ComboPointType::dodge]	= 0;
	combo_point_change[gc::ComboPointType::disrupt]	= 0;

	name = "Snipe";
	initial_desc = get_owner().get_name() + " takes a careful shot at the enemy...";
}
