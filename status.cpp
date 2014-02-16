#include "status.h"
#include "gameunit.h"
#include "abilities.h"
#include <iostream>

void AttributeListGroup::add_attribute_list(AttributeList attr, bool self_target, int efc){
	attributes.push_back(attr);
	targeting_logic.push_back(self_target);
	effect_chances.push_back(efc);
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
	stat->status_list = this;
	statuses[stat->get_type()].push_back(stat);

}

void StatusList::remove_status(Status *stat) {
	auto it = stat->status_list->statuses.find(stat->get_type());
	for (size_t i = 0; i < it->second.size(); ++i){
		if (it->second[i] == stat){
			delete stat;
			it->second.erase(it->second.begin() + i);
		}
	}
	if (DEBUG_STATUS){
		std::cout << "REMOVE STATUS OKAY" << std::endl;
		system("pause");
	}
}
void StatusList::remove_statuses_of_type(gc::StatusType stat_type){
	auto i = statuses.find(stat_type);
	try {
		if (i == statuses.end()){
			std::exception e("Could not find statuses of this stat type");
			throw e;
		}
		for (size_t j = 0; j < i->second.size(); ++j){
			for (size_t k = 0; k < i->second[j]->linked_statuses.size(); ++k){
				remove_status(i->second[j]->linked_statuses[0]);
				i->second[j]->linked_statuses.erase(i->second[j]->linked_statuses.begin());
			}
			delete i->second[j];
			i->second.erase(i->second.begin() + j);
			std::cout << i->second.size() << std::endl;
			std::cout << int(i->first) << std::endl;
			system("pause");
		}
	}
	catch (std::exception &ex) {
		std::cout << ex.what() << std::endl;
		system("pause");
	}
}
void StatusList::update(){
	for (auto i = statuses.begin(); i != statuses.end(); ++i){
		for (size_t j = 0; j < i->second.size(); ++j){
			if (!i->second[j]->update(gu)){
				for (size_t k = 0; k < i->second[j]->linked_statuses.size(); ++k){
					remove_status(i->second[j]->linked_statuses[0]);
					i->second[j]->linked_statuses.erase(i->second[j]->linked_statuses.begin());
				}
				delete i->second[j];
				i->second.erase(i->second.begin() + j);
			}
		}
	}
}
void StatusList::empty() {
	for (auto i = statuses.begin(); i != statuses.end(); ++i){
		for (size_t j = 0; j < i->second.size(); ++j){
			int ls_size = int(i->second[j]->linked_statuses.size());
			for (int k = 0; k < ls_size; ++k){
				remove_status(i->second[j]->linked_statuses[0]);
				i->second[j]->linked_statuses.erase(i->second[j]->linked_statuses.begin());
			}
			delete i->second[j];
			i->second.erase(i->second.begin() + j);
		}
	}
}

Status::Status(AttributeList attr, Ability *a) : attr_list(attr), from_ability(a), timer(0) {
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
	this->linked_statuses.push_back(ls);
	ls->linked_statuses.push_back(this);
}
