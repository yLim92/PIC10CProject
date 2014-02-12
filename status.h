#ifndef STATUS_H
#define STATUS_H

#include "utility.h" //gc::StatusType
#include <map>

class GameUnit;
class Status;

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

struct StatusBlock {
	StatusBlock();
	~StatusBlock() {}

	AttributeList attr_list;
	std::vector<AttributeList> l_attr_list;
	bool main_self_target;
	bool linked_self_target;
	int effect_chance;
};

class StatusList {
public:
	StatusList() {}
	StatusList(GameUnit* g) : gu(g) {}
	virtual ~StatusList() {}

	const std::map<gc::StatusType, double> get_all_status_and_values() const;
	const double get_status_mod(gc::StatusType st) const;
	const bool is_affected_by(gc::StatusType st) const;
	void add_status(Status *stat);
	void remove_status(Status *stat);
	void update();
	void empty();
protected:
	std::map<gc::StatusType, std::vector<Status*> > statuses;
	GameUnit* gu;
};

class Status {
	friend class Ability;
	friend class StatusList;
public:
	Status() {};
	Status(AttributeList attr, Ability *a); 
	virtual ~Status();

	const gc::StatusType get_type() const { return attr_list.type; }
	double mod() const;
	//Returns 0 if the status should no longer have an effect
	virtual int update(GameUnit* gu);

	virtual void link_status(Status* ls);
protected:
	AttributeList attr_list;
	Ability* from_ability;
	std::vector<Status*> linked_statuses;
	StatusList* status_list;

	int timer;

};


#endif //STATUS_H