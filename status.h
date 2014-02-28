#ifndef STATUS_H
#define STATUS_H

#include "utility.h" //gc::StatusType, gc::TargetType
#include <map>

class GameUnit;
class Status;
class Ability;
struct LinkedStatus;
class BattleView;

struct AttributeList {
	AttributeList() : is_stackable(true), effect_decay(0), effect_magnitude(0), duration(gc::PRACTICALLY_INFINITY), interval(1*gc::FPS), sustain_chance(0), sustain_chance_decay(0), type(gc::StatusType::undefined) {}
	~AttributeList() {}

	bool is_stackable;
	double effect_decay;
	double effect_magnitude;
	int interval;
	int duration;
	double sustain_chance;
	double sustain_chance_decay;
	gc::StatusType type;
};

class AttributeListGroup {
public:
	AttributeListGroup() {}
	AttributeListGroup(int ma) : main_effect_accuracy(ma) {}
	~AttributeListGroup() {}

	void add_attribute_list(AttributeList attr, bool self_target, int efc, std::string k);
	void set_link_logic(std::vector<std::pair<int, int> > ll) { link_logic = ll; }

	const AttributeList& get_attribute_list(int i) { return attributes[i]; }
	const bool get_target_logic(int i) const { return targeting_logic[i]; }
	const int get_effect_chance (int i) const { return effect_chances[i]; }
	const std::string get_id_key (int i) const { return id_keys[i]; }
	const int get_main_effect_accuracy() const { return main_effect_accuracy; }
	const std::vector<std::pair<int, int> >& get_link_logic() const { return link_logic; }

	const int get_status_count() const { return int(attributes.size()); }
private:
	std::vector<AttributeList> attributes;
	std::vector<bool> targeting_logic;
	std::vector<int> effect_chances;
	std::vector<std::pair<int, int> > link_logic;
	std::vector<std::string> id_keys;
	int main_effect_accuracy;
};

class StatusList {
public:
	StatusList() {}
	StatusList(GameUnit* g) : gu(g) {}
	virtual ~StatusList() {}

	const std::map<gc::StatusType, double> get_all_status_and_values() const;
	const double get_status_mod(gc::StatusType st) const;
	const bool is_affected_by(gc::StatusType st) const;
	const Status* get_last_status_of_type(gc::StatusType st) const;
	void add_status(Status *stat);
	//void remove_status(Status *stat);
	void remove_individual_status(gc::StatusType st, string k);
	void remove_statuses_of_type(gc::StatusType stat_type);
	void remove_status_and_links(gc::StatusType st, string k);
	void update();
	void empty();
	void remove_linked_statuses(Status &stat); 

	void do_ability_statuses(gc::StatusType st, GameUnit &tgt, int magn, double mult, BattleView &bv, Ability &trig_abl, GameUnit &current);

	void update_duplicate_status(Status &existing, Status &updater) const;
	Status* get_status(gc::StatusType st, string k);
protected:
	std::map<gc::StatusType, std::vector<Status*> > statuses;
	GameUnit* gu;
};

class Status {
	friend class Ability;
	friend class StatusList;
public:
	Status() {};
	Status(AttributeList attr, Ability *a, std::string k, StatusList &sl); 
	virtual ~Status();


	const gc::StatusType get_type() const { return attr_list.type; }
	const Ability& get_creating_ability() const { return *from_ability; }
	const GameUnit& get_source() const;
	double mod() const;
	//Returns 0 if the status should no longer have an effect
	virtual int update(GameUnit* gu);

	virtual void link_status(Status* ls);
protected:
	AttributeList attr_list;
	Ability* from_ability;
	vector<LinkedStatus> linked_statuses;
	StatusList* status_list;

	int timer;
	std::string key;
};

struct LinkedStatus {
	LinkedStatus() {}
	LinkedStatus(StatusList &sl, string k, gc::StatusType st) : s_list(&sl), key(k), type(st) {}

	StatusList *s_list;
	string key;
	gc::StatusType type;
};

class EquipmentList {
public:
	EquipmentList() {}
	EquipmentList(GameUnit *gu) : owner(gu) {}
	virtual ~EquipmentList() {}

private:
	GameUnit *owner;
	
};


class Equipment {
public:
	Equipment() {}
	Equipment(int ilvl, int rare_bonus);
	virtual ~Equipment() {}

	int generate_mod(gc::EquipMod eqmod);
	virtual const int base_ability_mod() const { return item_level + 4; }
	virtual const int base_accuracy_mod() const { return (item_level + 9)/2; }
	virtual  int base_armor_mod() const { return item_level * 5; }
	virtual const int base_armor_pierce_mod() const { return 1 + (item_level)/5; }
	virtual const int base_stat_mod() const { return item_level * 2; }
	virtual const int base_crit_chance_mod() const { return 1 + item_level/10; }
	virtual const int base_crit_damage_mod() const { return 50 + (item_level-1)*3; }
	virtual const int base_damage_reduct_mod() const { return 1 + (item_level)/5; }
	virtual const int base_dodge_mod() const { return (item_level + 9)/2; }
	virtual const int base_effect_chance_mod() const { return 1 + (item_level * 2) / 5; }
	virtual const int base_effect_resist_mod() const { return 1 + (item_level * 2) / 5; }
	virtual const int base_experience_mod() const { return 1 + (item_level) / 5; }
	virtual const int base_gold_mod() const { return 1 + (item_level) / 10; }
	virtual const int base_initial_turn_mod() const { return 10 + item_level; }
	virtual const int base_max_hp_mod() const { return 10 * item_level; }
	virtual const int base_rare_item_mod() const { return 5 * item_level; }
	virtual const int base_regen_resource_mod() const { return 5 + item_level - 1; }
	virtual const int base_hp_regen_mod() const { return 1 + item_level/5; }
	virtual const int base_shield_mod() const { return 20 + item_level * 4; }
	virtual const int base_speed_mod() const { return 5 + item_level - 1; }
	virtual const int base_vampirism_mod() const { return 1 + item_level/2; }

protected:
	int item_level,
		gold_value;

	gc::EquipRarity rarity;
	string name;

};

class MainHandEquip : public Equipment {
public:
	MainHandEquip() {}
	virtual ~MainHandEquip() {}


private:

};
class BodyEquip : public Equipment {
public:
	BodyEquip() {}
	virtual ~BodyEquip() {}

private:

};
class HelmetEquip : public Equipment {
public:
	HelmetEquip() {}
	virtual ~HelmetEquip() {}


private:

};
class BootsEquip : public Equipment {
public:
	BootsEquip() {}
	virtual ~BootsEquip() {}

private:

};


#endif //STATUS_H