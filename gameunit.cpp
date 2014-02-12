#include "gameunit.h"
#include "utility.h"
#include "abilities.h"

#include <iostream>

GameUnit::GameUnit() {
	initialize_stats();

	temp_hp = get_intial_temp_hp();
	assign_abilities();
}

GameUnit::GameUnit(std::string n) {
	initialize_stats();
	name = n;

	temp_hp = get_intial_temp_hp();
	assign_abilities();
}

GameUnit::GameUnit(int c, int s, int d, int i) {
	initialize_stats();
	base_con = c;
	base_str = s;
	base_dex = d;
	base_int = i;

	temp_hp = get_intial_temp_hp();
	assign_abilities();
}

GameUnit::GameUnit(std::string n, int c, int s, int d, int i) {
	initialize_stats();
	name = n;
	base_con = c;
	base_str = s;
	base_dex = d;
	base_int = i;

	temp_hp = get_intial_temp_hp();
	assign_abilities();
}

GameUnit::GameUnit(std::string n, int lvl) {
	initialize_stats();
	name = n;
	level = lvl;

	temp_hp = get_intial_temp_hp();
	assign_abilities();
}

GameUnit::~GameUnit() {

}
void GameUnit::initialize_stats() {
	level = 1;
	base_con = 10;
	base_str = 10;
	base_dex = 10;
	base_int = 10;
	base_turn_cycle	= 5*gc::FPS;
	base_accuracy = 0;
	base_dodge = 0;
	base_armor = 0;
	base_effect_accuracy = 0;
	base_effect_resist = 0;
	con_per_lvl = 1;
	str_per_lvl = 1;
	dex_per_lvl = 1;
	int_per_lvl = 1;
	damage_taken = 0;
	base_hp = 100;
	is_front = false;
	name = "";
	turn_increment = int(utility::normal_rng(gc::AVG_FRAMES_PER_TURN / 4.0, 4.0));
	status_list = StatusList(this);
}
void GameUnit::reset_defaults() {

}

void GameUnit::assign_abilities() {
	abilities.push_back(new Ability(this));
	abilities.push_back(new ChargeStrike(this));
}
const int GameUnit::get_constitution() const { 
	return base_con + int(level * con_per_lvl); 
}
const int GameUnit::get_strength() const { 
	return base_str + int(level * str_per_lvl); 
}
const int GameUnit::get_dexterity() const { 
	return base_dex + int(level * dex_per_lvl);
}
const int GameUnit::get_intelligence() const { 
	return base_int + int(level * int_per_lvl); 
}
const int GameUnit::get_speed() const { 
	return int(sqrt(get_dexterity())*(1. + get_status_mod(gc::StatusType::speed))); 
}
const double GameUnit::get_resource_mod() const {
	return (1.0 + double(sqrt(get_intelligence()))/100.0)*(1. + get_status_mod(gc::StatusType::regen_resource));
}
const int GameUnit::get_intial_temp_hp() const {
	return int(get_max_hp() * (0.2 + 0.8 * pow(1 - 100./(100. + get_strength()), 2) ) ); 
}
const int GameUnit::get_max_temp_hp() const {
	return int(get_max_hp() * (0.5 + 0.5 * pow(1 - 100./(100. + get_strength()), 2) ) ); 
}
const int GameUnit::get_max_hp() const { 
	return base_hp + get_constitution() * 10; 
}
const int GameUnit::get_accuracy() const{
	return int(base_accuracy + 100 * get_status_mod(gc::StatusType::accuracy) + 100 * (1 - 100./(100. + get_intelligence()) ));
}
const int GameUnit::get_dodge() const{
	return int(base_dodge + 100 * get_status_mod(gc::StatusType::dodge) + 100 * (1 - 100./(100. + get_dexterity()) ));
}
//Gets a FLAT %. Lowest is 0.
const int GameUnit::get_damage_reduction() const{
	return int(100 * get_status_mod(gc::StatusType::damage_reduction) + 100 * (1 - 100./(100. + get_constitution() + base_armor) ) );
}
const int GameUnit::get_effect_accuracy() const {
	return int(base_effect_accuracy + 100 * get_status_mod(gc::StatusType::accuracy) + 100 * pow(1 - 100./(100. + get_intelligence()), 2) );
}
const int GameUnit::get_effect_resist() const{
	return int(base_effect_resist + 100 * get_status_mod(gc::StatusType::effect_resist) + 100 * pow(1 - 100./(100. + get_constitution()), 2) );
}
//Gets a FLAT %. Lowest is 0.
const int GameUnit::get_armor_ignore() const{
	return int(sqrt(get_strength()) + 100 * get_status_mod(gc::StatusType::armor_pierce));
}
const int GameUnit::get_critical_chance() const{
	return int(sqrt(get_dexterity()) + 100 * get_status_mod(gc::StatusType::critical_chance));
}
const double GameUnit::get_critical_damage_mod() const{
	return 2.5 + get_status_mod(gc::StatusType::critical_damage);
}
const int GameUnit::get_hp() const {
	int hp = get_max_hp() - damage_taken;
	if (hp > 0)
		return hp;
	else
		return 0;
}
const double GameUnit::get_status_mod(gc::StatusType st) const {
	return status_list.get_status_mod(st);
}
const std::map<gc::StatusType, double> GameUnit::get_all_status_and_values() const {
	return status_list.get_all_status_and_values();
}
const int GameUnit::get_temp_hp() const {
	if (temp_hp > 0)
		return temp_hp;
	else
		return 0;
}

int GameUnit::change_hp(int c){
	temp_hp += c;
	if (temp_hp < 0) {
		damage_taken += (-temp_hp);
		temp_hp = 0;
	}
	else if (temp_hp > get_max_temp_hp())
		temp_hp = get_max_temp_hp();


	if (c <= 0)
		return -c;
	else
		return c;
}

bool GameUnit::is_defeated() {
	if (get_hp() > 0)
		return false;
	else
		return true;
}

void GameUnit::add_status(Status *stat) {
	status_list.add_status(stat);
}
void GameUnit::update() {
	if (is_defeated()){
		status_list.empty();
	}
	else {
		status_list.update();

		if (!status_list.is_affected_by(gc::StatusType::stun))
			++turn_increment;
	}
}
//If the turn progress is greater than 100%
bool GameUnit::is_turn() {
	if (get_turn_progress() < 1.00)
		return false;
	else {
		return true;
	}
}
const double GameUnit::get_turn_progress() const{ 
	return double(turn_increment) / double(base_turn_cycle - get_speed()); 
}

Ability* GameUnit::get_ability(int i) { 
	if (i >= int(abilities.size()))
		return nullptr;
	else
		return abilities[i]; 
}

Party::Party(std::vector<GameUnit*> p, gc::Affiliation a) : party_members(p), affiliation(a) {
	for (size_t i = 0; i < party_members.size(); i++)
		party_members[i]->set_affiliation(affiliation);
}
bool Party::is_party_defeated() {
	for (size_t i = 0; i < party_members.size(); ++i){
		if (!party_members[i]->is_defeated())
			return false;
	}
	return true;
}

PlayerUnit::~PlayerUnit() {

}

void PlayerUnit::update() {
	GameUnit::update();
	update_resource();
}
void PlayerUnit::update_resource() {
	return;
}

Npc::~Npc() {

}

Soldier::Soldier(std::string n, int lvl) : PlayerUnit(n, lvl) {
	base_con = 10;
	base_str = 15;
	base_dex = 5;
	base_int = 5;
	con_per_lvl = 1;
	str_per_lvl = 2;
	dex_per_lvl = 0.5;
	int_per_lvl = 0.5;
	assign_abilities();
}
Ability* Soldier::get_ability(int i) { 
	if (i >= int(w_abilities.size()))
		return nullptr;
	else
		return w_abilities[i]; 
}

void Soldier::assign_abilities() {
	w_abilities.push_back(new MoraleBoost(this));
	w_abilities.push_back(new Hold(this));
}
void Soldier::update_resource() {
	for (size_t i = 0; i < w_abilities.size(); ++i){
		w_abilities[i]->decrement_cd();
	}
}

Mage::Mage(std::string n, int lvl) : PlayerUnit(n, lvl) {
	base_con = 10;
	base_str = 5;
	base_dex = 5;
	base_int = 15;
	con_per_lvl = 1;
	str_per_lvl = 0.5;
	dex_per_lvl = 0.5;
	int_per_lvl = 2;

	base_mp_regen = 5.0/gc::FPS;
	current_mp = 0; 

	assign_abilities();
}
Ability* Mage::get_ability(int i) { 
	if (i >= int(m_abilities.size()))
		return nullptr;
	else
		return m_abilities[i]; 
}

void Mage::assign_abilities() {
	m_abilities.push_back(new Fireball(this));
	m_abilities.push_back(new Meteor(this));
}
void Mage::update_resource() {
	current_mp += base_mp_regen*get_resource_mod();
}
void Mage::change_mp(int change){
	current_mp += double(change);
	if (current_mp < 0)
		current_mp = 0;
}
Thief::Thief(std::string n, int lvl) : PlayerUnit(n, lvl) {
	base_con = 10;
	base_str = 5;
	base_dex = 15;
	base_int = 5;
	con_per_lvl = 1;
	str_per_lvl = 0.5;
	dex_per_lvl = 2;
	int_per_lvl = 0.5;

	combo_points[gc::ComboPointType::attack] = 1;
	combo_points[gc::ComboPointType::dodge]	= 1;
	combo_points[gc::ComboPointType::disrupt] = 1;

	assign_abilities();
}

Ability* Thief::get_ability(int i){
	if (i >= int(t_abilities.size()))
		return nullptr;
	else
		return t_abilities[i]; 
}

void Thief::assign_abilities() {
	t_abilities.push_back(new RogueAttack(this));
	t_abilities.push_back(new Snipe(this));
}
void Thief::update_resource(){
	return;
}
void Thief::change_combo_points(gc::ComboPoints cp){
	for (auto it = cp.begin(); it != cp.end(); ++it){
		if (it->second > 0){
			int bonus_chance = int(get_resource_mod())*100 - 100;
			while(bonus_chance > utility::rng(0, 99)){
				++combo_points[it->first];
				bonus_chance -= 100;
			}
		}
		combo_points[it->first] += it->second;
	}
		
}

const std::string Thief::get_resource_display() const { 
	std::string s = "";
	for (auto it = combo_points.begin(); it != combo_points.end(); ++it)
		s += gc::combo_point_name(it->first) + std::to_string(it->second) + " ";
	return s;
}
 const gc::ComboPoints& Thief::get_combo_points() const { 
	return combo_points;
 }