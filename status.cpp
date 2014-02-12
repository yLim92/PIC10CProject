#include "status.h"
#include "gameunit.h"
#include <iostream>

StatusBlock::StatusBlock() {
	attr_list = AttributeList();
	l_attr_list.resize(0);
	main_self_target = false;
	linked_self_target = false;
	effect_chance = 0;
}

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

void StatusList::add_status(Status *stat) {
	stat->status_list = this;
	statuses[stat->get_type()].push_back(stat);

}

#include "abilities.h"
void StatusList::remove_status(Status *stat) {
	auto it = stat->status_list->statuses.find(stat->get_type());
	for (size_t i = 0; i < it->second.size(); ++i){
		if (it->second[i] == stat){
			delete stat;
			it->second.erase(it->second.begin() + i);
		}
	}
}
void StatusList::update(){
	for (auto i = statuses.begin(); i != statuses.end(); ++i){
		for (size_t j = 0; j < i->second.size(); ++j){
			if (!i->second[j]->update(gu)){
				for (size_t k = 0; k < i->second[j]->linked_statuses.size(); ++k)
					remove_status(i->second[j]->linked_statuses[k]);
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
	return attr_list.effect_magnitude*(1.0 - timer*attr_list.effect_decay/gc::FPS);
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
