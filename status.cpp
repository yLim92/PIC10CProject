#include "status.h"
#include "gameunit.h"
#include "abilities.h"
#include <iostream>

void AttributeListGroup::add_attribute_list(AttributeList attr, bool self_target, int efc, std::string k){
	attributes.push_back(attr);
	targeting_logic.push_back(self_target);
	effect_chances.push_back(efc);
	id_keys.push_back(k);
}
//Status mods can be negative.  Mods should be in decimals percents (i.e. 0.08 = 8%)
const double StatusList::get_status_mod(gc::StatusType st) const {
	double total = 0;
	auto it = statuses.find(st);
	if (it != statuses.end()){
		for (size_t i = 0; i < it->second.size(); ++i){
			total += it->second[i]->mod();
		}
	}
	return total;
}
const bool StatusList::is_affected_by(gc::StatusType st) const{
	auto it = statuses.find(st);
	if (it != statuses.end()){
		if (it->second.size() > 0)
			return true;
	}
	return false;
}

const std::map<gc::StatusType, double> StatusList::get_all_status_and_values() const {
	std::map<gc::StatusType, double> sm;
	for (auto it = statuses.begin(); it != statuses.end(); ++it){
		if (it->second.size() > 0)
			sm[it->first] = get_status_mod(it->first);
	}
	return sm;
}
const Status* StatusList::get_last_status_of_type(gc::StatusType st) const {
	auto it = statuses.find(st);
	if (it != statuses.end()){
		if (int(it->second.size()) > 0)
			return it->second.back();
		else
			return nullptr;
	}
	return nullptr;
}

void StatusList::add_status(Status *stat) {
	Status *dupl = get_status(stat->get_type(), stat->key);
	if (dupl != nullptr){
		update_duplicate_status(*dupl, *stat);
	}
	else {
		statuses[stat->get_type()].push_back(stat);
	}
}
//Need to adjust if there are two or more statuses that are not automatically applied; maybe a shared ptr to a common linked status container?
void StatusList::update_duplicate_status(Status &ex, Status &up) const{
	ex.attr_list.duration = (ex.attr_list.duration > up.attr_list.duration) ? ex.attr_list.duration : up.attr_list.duration;
	ex.attr_list.effect_decay = (ex.attr_list.effect_decay < up.attr_list.effect_decay) ? ex.attr_list.effect_decay : up.attr_list.effect_decay;
	ex.attr_list.effect_magnitude = (abs(ex.attr_list.effect_magnitude) > abs(up.attr_list.effect_magnitude)) ? ex.attr_list.effect_magnitude : up.attr_list.effect_magnitude;
	ex.attr_list.interval = (ex.attr_list.interval < up.attr_list.interval) ? ex.attr_list.interval : up.attr_list.interval;
	ex.attr_list.sustain_chance = (ex.attr_list.sustain_chance > up.attr_list.sustain_chance) ? ex.attr_list.sustain_chance : up.attr_list.sustain_chance;
	ex.attr_list.sustain_chance_decay = (ex.attr_list.sustain_chance_decay < up.attr_list.sustain_chance_decay) ? ex.attr_list.sustain_chance_decay : up.attr_list.sustain_chance_decay;
	
	ex.timer = 0;
	
	if (ex.linked_statuses.size() < up.linked_statuses.size()){
		for (size_t i = 0; i < up.linked_statuses.size(); ++i){
			bool found = false;
			for (size_t j = 0; j < ex.linked_statuses.size(); ++j){
				if (up.linked_statuses[i].key == ex.linked_statuses[j].key){
					found = true;
					break;
				}
			}
			if (!found)
				ex.linked_statuses.push_back(up.linked_statuses[i]);
		}
	}

	delete &up;
}
Status* StatusList::get_status(gc::StatusType st, string k){
	auto it = statuses.find(st);
	if (it != statuses.end()){
		for (size_t i = 0; i < it->second.size(); ++i){
			if (k == it->second[i]->key){
				return it->second[i];
			}
		}
	}
	return nullptr;
}
void StatusList::remove_individual_status(gc::StatusType st, string k) {
	auto it = statuses.find(st);
	if (it != statuses.end()){
		for (size_t i = 0; i < it->second.size(); ++i){
			if (it->second[i]->key == k){
				delete it->second[i];
				it->second.erase(it->second.begin() + i);
				return;
			}
		}
	}
}
void StatusList::remove_statuses_of_type(gc::StatusType stat_type){
	auto i = statuses.find(stat_type);
	if (i == statuses.end())
		return;
	while (int(i->second.size()) > 0){
		remove_linked_statuses(*i->second[0]);
		delete i->second[0];
		i->second.erase(i->second.begin());
	}
}

void StatusList::update(){
	for (auto i = statuses.begin(); i != statuses.end(); ++i){
		for (size_t j = 0; j < i->second.size();){
			if (!i->second[j]->update(gu)){
				remove_linked_statuses(*i->second[j]);
				delete i->second[j];
				i->second.erase(i->second.begin() + j);
			}
			else
				 ++j;
		}
	}
}
void StatusList::empty() {
	for (auto i = statuses.begin(); i != statuses.end(); ++i){
		while (int(i->second.size()) > 0){
			remove_linked_statuses(*i->second[0]);
			delete i->second[0];
			i->second.erase(i->second.begin());
		}
		i->second.clear();
	}
}
void StatusList::remove_linked_statuses(Status &stat){
	int ls_size = int(stat.linked_statuses.size());
	for (int i = 0; i < ls_size; ++i){
		LinkedStatus ls = stat.linked_statuses[0];
		ls.s_list->remove_individual_status(ls.type, ls.key);
		stat.linked_statuses.erase(stat.linked_statuses.begin());
	}
}

void StatusList::remove_status_and_links(gc::StatusType st, string k) {
	Status *stat = get_status(st, k);
	try {
		if (stat == nullptr){
			std::exception ex("Could not find stat to remove.");
			throw ex;
		}
	}
	catch (std::exception &e){
		cout << e.what() << endl;
		system("pause");
		return;
	}

	remove_linked_statuses(*stat);
	remove_individual_status(st, k);
}

void StatusList::do_ability_statuses(gc::StatusType st, GameUnit &tgt, int magn, double mult, BattleView &bv, Ability &trig_abl, GameUnit &current){
	if (st == gc::StatusType::ability_status_on_dmg || st == gc::StatusType::ability_status_on_hit){
		auto it = statuses.find(st);
		if (it != statuses.end()){
			for (int j = 0; j < int(it->second.size()); ++j){
				it->second[j]->from_ability->do_ability_as_status(st, tgt, magn, mult, bv, trig_abl, current);
			}
		}
	}
}


Status::Status(AttributeList attr, Ability *a, std::string k, StatusList &sl) : attr_list(attr), from_ability(a), timer(0), key(k), status_list(&sl) {
}
Status::~Status(){
}
double Status::mod() const{
	/*
	if (get_type() == gc::StatusType::channelling ||
		get_type() == gc::StatusType::stun)
		return 1;*/
	return attr_list.effect_magnitude*(1.0 - timer*attr_list.effect_decay/gc::FPS);
}

const GameUnit& Status::get_source() const { 
	return from_ability->get_owner(); 
}
int Status::update(GameUnit* gu) {
	using namespace gc;

	++timer;
	if (timer % attr_list.interval == 0){
		if (attr_list.type == StatusType::burn || attr_list.type == StatusType::bleed || attr_list.type == StatusType::poison)
			gu->change_hp(int(-mod()));
		else if (attr_list.type == StatusType::regen_temp_hp)
			gu->change_hp(int(mod()));
	}
	if (timer % attr_list.duration == 0){
		if (attr_list.sustain_chance*(1.0 - timer*attr_list.sustain_chance_decay / gc::FPS) < double(utility::rng(100)) / 100.0)
			return 0;
	}
	return 1;
}
void Status::link_status(Status* ls){
	this->linked_statuses.push_back(LinkedStatus(*ls->status_list, ls->key, ls->get_type()));
	ls->linked_statuses.push_back(LinkedStatus(*this->status_list, this->key, this->get_type()));
}
