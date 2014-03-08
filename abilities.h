/**
Issue: What if ability is a charge ability and used more than once
In the future, should try to namespace all the abilities
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
class Npc;

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

	/* Ability phases go like this:
		1. Target step (target_step: set_possible_targets -> select_targets)
		2. Print attack flavor text (usage_desc)
		3. Calculate pre-ability things like dodge
		4. Apply effect if successful (initial_effect_step: apply_initial_effect -> apply_status
	*/
	virtual int do_ability_phase(std::vector<GameUnit*> &combatants, BattleView &bv);

	bool target_step(std::vector<GameUnit*> &combatants, std::vector<GameUnit *> &targets);
	virtual void set_possible_targets(std::vector<GameUnit*> &combatants, std::vector<GameUnit *> &possible_tgts);
	virtual bool select_targets(std::vector<GameUnit *> &poss_tgts, std::vector<GameUnit *> &targets);

	virtual void deduct_ability_cost() {}

	virtual void effect_step(std::vector<GameUnit *> &targets, BattleView &bv);
	virtual void do_harmful_effect(GameUnit &tgt, BattleView &bv, bool does_damage, bool does_status, bool do_post_phase);
	virtual void damage_step(GameUnit &tgt, double &magn, BattleView &bv);
	virtual void set_pre_hit_values(GameUnit &tgt, bool &hit, double &magn, double &mult, BattleView &bv);
	void print_health_change(GameUnit *tgt, int magn, BattleView &bv);
	virtual void apply_statuses(GameUnit &tgt, double mult);
	virtual void post_attack_phase(bool hit, GameUnit &tgt, int magn, double mult, BattleView &bv);
	virtual void do_ability_as_status(gc::StatusType, GameUnit &tgt, int magn, double mult, BattleView &bv, Ability &trig_abl, GameUnit &current) {}
	
	void do_delayed_ability(std::vector<GameUnit *> &targets, BattleView &bv);

	virtual const int effect_magnitude() const;
	const int mod_only_magnitude() const;
	const int con_only_magnitude() const;
	const int str_only_magnitude() const;
	const int int_only_magnitude() const;
	const int dex_only_magnitude() const;
	virtual const double effect_spread(bool perfect) const;
	const int critical_chance() const;
	const double critical_damage_multiplier() const;
	virtual const int accuracy() const;
	//const int status_effect_chance() const;
	const int get_delay() const { return delay; }
	virtual const std::string get_name() const { return name; }
	const bool is_status_successful(int base_acc, double mult, GameUnit &tgt) const;

	virtual GameUnit& get_owner() { return *owner; }
	virtual const GameUnit& get_owner() const { return *owner; }

	virtual const std::vector<AttributeListGroup> status_templates() const;
	const void link_all(vector<pair<int, int> > &ll, int n) const;

	virtual const std::string get_cost_display() const { return ""; } 

	const std::string get_initial_description() const { return initial_desc; }
	const std::string get_delayed_description() const { return delayed_desc; }

	virtual const bool is_usable() const { return true; }
	const bool is_friendly() const;
	const bool is_single_target() const;
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
	
	std::vector<std::string> status_keys;
};

class ChargeStrike : public Ability {
public:
	ChargeStrike();
	ChargeStrike(GameUnit *g);
	virtual ~ChargeStrike() {}
	
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
	virtual void start_sustaining();
	virtual void stop_sustaining();

protected:
	int base_mp_cost;
	Mage *mage;
	bool is_sustaining;
	std::pair<gc::StatusType, string> sustain_status;
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

class NpcAbility : public Ability {
public: 
	NpcAbility(Npc *o);
	virtual const GameUnit& get_owner() const;
	virtual GameUnit& get_owner();

protected:
	Npc *npc;
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
	virtual void do_ability_as_status(gc::StatusType st, GameUnit &tgt, int magn, double mult, BattleView &bv, Ability &trig_abl, GameUnit &current);
};
class Batter: public WarriorAbility {
public:
	Batter(Soldier *o);
	virtual ~Batter() {}

	virtual const bool is_usable() const; 
	virtual void decrement_cd();
	virtual void post_attack_phase(bool hit, GameUnit &tgt, int magn, double mult, BattleView &bv);

	virtual const std::vector<AttributeListGroup> status_templates() const;
	virtual const int accuracy() const;
private:
	int free_usage_timer;
	int consecutive_uses;
};
class Cleave: public WarriorAbility {
public:
	Cleave(Soldier *o);
	virtual ~Cleave() {}

	virtual bool select_targets(std::vector<GameUnit *> &poss_tgts, std::vector<GameUnit *> &targets);
};
class Resolve: public WarriorAbility {
public:
	Resolve(Soldier *o);
	virtual ~Resolve() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
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

protected:
};

class Bolt : public MageAbility {
public:
	Bolt(Mage *o);
	virtual ~Bolt() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
};

class LightningShield : public MageAbility {
public:
	LightningShield(Mage *o);
	virtual ~LightningShield() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
	virtual void do_ability_as_status(gc::StatusType st, GameUnit &tgt, int magn, double mult, BattleView &bv, Ability &trig_abl, GameUnit &current);
};

class SummonFamiliar : public MageAbility {
public:
	SummonFamiliar(Mage *o);
	virtual ~SummonFamiliar() {}

	
};

class Barrier : public MageAbility {
public:
	Barrier(Mage *o);
	virtual ~Barrier() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
	virtual void apply_statuses(GameUnit &tgt, double mult);
};

class Curse : public MageAbility {
public:
	Curse(Mage *o);
	virtual ~Curse() {}

	virtual void post_attack_phase(bool hit, GameUnit &tgt, int magn, double mult, BattleView &bv);
	virtual const std::vector<AttributeListGroup> status_templates() const;
};

class Enfeeble : public MageAbility {
public:
	Enfeeble(Mage *o);
	virtual ~Enfeeble() {}

	virtual void post_attack_phase(bool hit, GameUnit &tgt, int magn, double mult, BattleView &bv);
	virtual const std::vector<AttributeListGroup> status_templates() const;
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

/*
	DRACOZOMBIE ABILITIES
*/

class ClawAttack : public NpcAbility {
public:
	ClawAttack(Npc *o);
	virtual ~ClawAttack() {}
};

class RottingBreath : public NpcAbility {
public:
	RottingBreath(Npc *o);
	virtual ~RottingBreath() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
};

class Consume : public NpcAbility {
public:
	Consume(Npc *o);
	virtual ~Consume() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;

	virtual const int effect_magnitude() const { return str_only_magnitude(); }
};

class DecayCurse : public NpcAbility {
public:
	DecayCurse(Npc *o);
	virtual ~DecayCurse() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
};

/* Cultist Abilities */

class MagicBolt : public NpcAbility {
public:
	MagicBolt(Npc *o);
	virtual ~MagicBolt() {}

	virtual const std::vector<AttributeListGroup> status_templates() const;
};

class Frenzy : public NpcAbility {
public:
	Frenzy(Npc *o);
	virtual ~Frenzy() {}

	virtual void set_possible_targets(std::vector<GameUnit*> &combatants, std::vector<GameUnit *> &possible_tgts);
	virtual const std::vector<AttributeListGroup> status_templates() const;
};


#endif