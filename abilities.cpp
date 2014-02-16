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
void Ability::apply_statuses(GameUnit &tgt, double mult){
	std::vector<AttributeListGroup> algs = status_templates();
	for (int i = 0; i < int(algs.size()); ++i) {
		if (is_status_successful(algs[i].get_main_effect_accuracy(), mult, tgt)){
			std::vector<std::pair<Status*, bool> > statuses;
			AttributeListGroup attr_group = algs[i];
			int sz = attr_group.get_status_count();
			for (int j = 0; j < sz; ++j){
				
				if (is_status_successful(attr_group.get_effect_chance(j), mult, tgt))
					statuses.push_back(std::make_pair(new Status(attr_group.get_attribute_list(j), this), attr_group.get_target_logic(j)) );
				else
					statuses.push_back(std::make_pair(nullptr, false));
				
			}
			std::vector<std::pair<int, int> > ll = attr_group.get_link_logic();
			for (int j = 0; j < int(ll.size()); ++j){
				//If the statuses (statuses.first) at index ll.first and ll.second are not nullptrs, link them
				if (statuses[ll[j].first].first != nullptr &&  statuses[ll[j].second].first != nullptr)
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
const bool Ability::is_friendly() const { 
	if (target_type == gc::TargetType::self ||
		target_type == gc::TargetType::single_friendly ||
		target_type == gc::TargetType::party_friendly)
		return true;
	else
		return false;
}
/*

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
const std::vector<AttributeListGroup> ChargeStrike::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::channelling;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY);

	v_algs.push_back(alg);
	return v_algs;
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

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::ability;
	attr.duration = 10 * gc::FPS;
	attr.effect_magnitude = 0.5;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY);

	v_algs.push_back(alg);
	return v_algs;
}

Hold::Hold(Soldier* o) : WarriorAbility(o) {
	health_effect = false;
	status_effect = true;

	base_accuracy = 70;

	base_cd = 2.0 * gc::FPS;

	name = "Restrain";
	initial_desc = warrior->get_name() + " attempts to grab the target...";

}
const std::vector<AttributeListGroup> Hold::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::channelling;
	attr.duration = 10 * gc::FPS;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY);

	attr = AttributeList();
	attr.type = gc::StatusType::grab;
	attr.duration = 1 * gc::FPS;
	attr.sustain_chance = 0.8;
	attr.sustain_chance_decay = 0.05;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY);
	/*
	attr = AttributeList();
	attr.type = gc::StatusType::poison;
	attr.duration = gc::PRACTICALLY_INFINITY;
	attr.effect_magnitude = 50;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY);*/

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

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::challenged;
	attr.duration = 20 * gc::FPS;
	attr.effect_magnitude = int(gc::Threat::low);
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY);

	v_algs.push_back(alg);
	return v_algs;
}

Retaliate::Retaliate(Soldier* o) : WarriorAbility(o) {
	damaging = false;
	health_effect = false;
	status_effect = true;
	target_type = gc::TargetType::self;

	con_effect_mod = 0.5;
	str_effect_mod = 1.0;

	base_cd = 30.0 * gc::FPS;

	name = "Retaliate";
	initial_desc = warrior->get_name() + " prepares to retaliate...";
}
const std::vector<AttributeListGroup> Retaliate::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::counter;
	attr.duration = 15 *gc::FPS;
	attr.effect_magnitude = effect_magnitude();
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY);

	v_algs.push_back(alg);
	return v_algs;
}
Batter::Batter(Soldier* o) : WarriorAbility(o) {
	status_effect = true;

	str_effect_mod = 1.5;
	base_accuracy = 80;

	base_cd = 30.0 * gc::FPS;
	

	free_usage_timer = 0;

	name = "Batter";
	initial_desc = warrior->get_name() + " strikes relentlessly!";
}
const std::vector<AttributeListGroup> Batter::status_templates() const {
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::speed;
	attr.duration = 5 *gc::FPS;
	attr.effect_magnitude = 0.1;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY);

	v_algs.push_back(alg);
	return v_algs;
}
void Batter::decrement_cd() {
	if (free_usage_timer > 0)
		free_usage_timer--;
	else
		remaining_cd--;
}
const bool Batter::is_usable() const{
	if(remaining_cd <= 0 || free_usage_timer > 0)
		return true;
	else
		return false;
}
void Batter::on_successful_hit() {
	free_usage_timer = 5*gc::FPS;
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

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::burn;
	attr.duration = 1 * gc::FPS;
	attr.effect_magnitude = effect_magnitude()*0.5;
	attr.effect_decay = 0.1;
	attr.sustain_chance = 1.0;
	attr.sustain_chance_decay = 0.2;
	alg.add_attribute_list(attr, false, gc::PRACTICALLY_INFINITY);

	v_algs.push_back(alg);
	return v_algs;
}

Meteor::Meteor(Mage *o) : MageAbility(o) {
	delay = 4 * gc::FPS;
	status_effect = true;

	int_effect_mod = 4;

	base_accuracy = 80;
	base_low_bound = 50;

	base_mp_cost = 5;

	name = "Meteor";
	initial_desc = get_owner().get_name() + " begins channelling...";
	delayed_desc = "A meteor crashes onto the battlefield!";
}

const std::vector<AttributeListGroup> Meteor::status_templates() const{
	std::vector<AttributeListGroup> v_algs;
	AttributeListGroup alg = AttributeListGroup(gc::PRACTICALLY_INFINITY);

	AttributeList attr = AttributeList();
	attr.type = gc::StatusType::channelling;
	alg.add_attribute_list(attr, true, gc::PRACTICALLY_INFINITY);

	v_algs.push_back(alg);
	return v_algs;
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
