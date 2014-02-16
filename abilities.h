/**
Issue: What if ability is a charge ability and used more than once
**/

#ifndef ABILITIES_H
#define ABILITIES_H

#include <vector>
#include <string>
#include "utility.h" //For gc::ComboPointType

class BattlePhase;
class BattleView;
class GameUnit;
class Ability;
class Status;
struct AttributeList;
class AttributeListGroup;
class Soldier;
class Mage;
class Thief;

struct DelayedAbility {
	DelayedAbility(Ability& a, int d, std::vector<GameUnit*> t) : ability(&a), delay(d), targets(t) {}
	~DelayedAbility() {}

	Ability* ability;
	int delay;
	std::vector<GameUnit*> targets;
};

class Ability {
public:
	Ability();
	Ability(GameUnit *g);
	virtual ~Ability();

	virtual void set_defaults();

	virtual void deduct_ability_cost() {}
	virtual void apply_statuses(GameUnit &tgt, double mult);
	virtual void on_successful_hit() {}

	const int effect_magnitude() const;
	virtual const double effect_spread(bool perfect) const;
	const int critical_chance() const;
	const double critical_damage_multiplier() const;
	const int accuracy() const;
	//const int status_effect_chance() const;
	const int get_delay() const { return delay; }
	virtual const std::string get_name() const { return name; }
	const bool is_status_successful(int base_acc, double mult, GameUnit &tgt) const;

	virtual GameUnit& get_owner() { return *owner; }
	virtual const GameUnit& get_owner() const { return *owner; }

	virtual const std::vector<AttributeListGroup> status_templates() const;

	virtual const std::string get_cost_display() const { return ""; } 

	const std::string get_initial_description() const { return initial_desc; }
	const std::string get_delayed_description() const { return delayed_desc; }

	virtual const bool is_usable() const { return true; }
	const bool is_friendly() const;

	const bool changes_health() const { return health_effect; }
	const bool is_damaging() const { return damaging; }
	const bool changes_status() const { return status_effect; }
	const gc::TargetType get_target_type() const { return target_type; }

protected:
	GameUnit *owner;

	//Ability Modifiers
	double con_effect_mod;
	double str_effect_mod;
	double dex_effect_mod;
	double int_effect_mod;
	double crit_damage_mod;
	
	int base_crit_chance;
	int base_crit_dmg_mod;
	int base_accuracy;

	int base_low_bound;
	int base_up_bound;

	//Ability Text
	std::string name;
	std::string initial_desc;
	std::string delayed_desc;

	//Ability Properties
	int delay; //Delay in frames until move triggers; currently 10 frames/second
	//bool friendly;
	bool health_effect;
	bool damaging;
	bool status_effect;
	gc::TargetType target_type;

};

class ChargeStrike : public Ability {
public:
	ChargeStrike();
	ChargeStrike(GameUnit *g);
	virtual ~ChargeStrike() {}
	
	const std::vector<AttributeListGroup> ChargeStrike::status_templates() const;
protected:
};

class WarriorAbility : public Ability {
public:
	WarriorAbility(Soldier *o);

	virtual const GameUnit& get_owner() const;
	virtual GameUnit& get_owner();

	virtual const bool is_usable() const; 
	virtual const std::string get_cost_display() const;
	virtual void decrement_cd() { --remaining_cd; }
	virtual void deduct_ability_cost();
protected:
	double base_cd;
	double remaining_cd;

	Soldier* warrior;
};

class MageAbility : public Ability {
public:
	MageAbility(Mage *o);

	virtual const GameUnit& get_owner() const;
	virtual GameUnit& get_owner();

	virtual const bool is_usable() const; 
	virtual const std::string get_cost_display() const;
	virtual void deduct_ability_cost();
protected:
	int base_mp_cost;
	Mage *mage;
};

class RogueAbility : public Ability {
public:
	RogueAbility(Thief *o);

	virtual const GameUnit& get_owner() const;
	virtual GameUnit& get_owner();

	virtual const bool is_usable() const; 
	virtual const std::string get_cost_display() const;
	virtual void deduct_ability_cost();
protected:
	gc::ComboPoints combo_point_change;
	Thief *rogue;
};

/*
	WARRIOR ABILITIES
*/
class MoraleBoost : public WarriorAbility {
public:
	MoraleBoost(Soldier *o);
	virtual ~MoraleBoost() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
protected:
	
};

class Hold : public WarriorAbility {
public:
	Hold(Soldier *o);
	virtual ~Hold() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
protected:
};

class Provoke: public WarriorAbility {
public:
	Provoke(Soldier *o);
	virtual ~Provoke() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
};

class Retaliate: public WarriorAbility {
public:
	Retaliate(Soldier *o);
	virtual ~Retaliate() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
};
class Batter: public WarriorAbility {
public:
	Batter(Soldier *o);
	virtual ~Batter() {}

	virtual const bool is_usable() const; 
	virtual void decrement_cd();
	virtual void on_successful_hit();
	virtual const std::vector<AttributeListGroup> status_templates() const;
private:
	int free_usage_timer;
};
class Cleave: public WarriorAbility {
public:
	Cleave(Soldier *o);
	virtual ~Cleave() {}
};
class Resolve: public WarriorAbility {
public:
	Resolve(Soldier *o);
	virtual ~Resolve() {}
};
class Strike: public WarriorAbility {
public:
	Strike(Soldier *o);
	virtual ~Strike() {}
};
/*
	MAGE ABILITIES
*/

class Fireball : public MageAbility {
public:
	Fireball(Mage *o);
	virtual ~Fireball() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
protected:
};

class Meteor : public MageAbility {
public:
	Meteor(Mage *o);
	virtual ~Meteor() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
protected:
};

/*
	ROGUE ABILITIES
*/

class RogueAttack : public RogueAbility {
public:
	RogueAttack(Thief *o);
	virtual ~RogueAttack() {}
protected:
};

class Snipe : public RogueAbility {
public:
	Snipe(Thief *o);
	virtual ~Snipe() {}
protected:
};


#endif