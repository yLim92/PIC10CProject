#include <vector>
#include <string>
#include "utility.h" //for gc::Affiliation
#include "status.h"

class BattlePhase;
class Ability;
class WarriorAbility;
class MageAbility;
class RogueAbility;
class Party;
class Turn;

class GameUnit {
	friend class Party;
public:
	GameUnit();
	GameUnit(std::string n);
	GameUnit(int c, int s, int d, int i); 
	GameUnit(std::string n, int c, int s, int d, int i);
	GameUnit(std::string n, int lvl);
	virtual ~GameUnit();

	virtual void initialize_stats();
	virtual void reset_defaults();
	virtual void assign_abilities();

	const std::string get_name() const { return name; }
	const std::vector<Ability *>& get_abilities() const { return abilities; }
	const int get_constitution() const;
	const int get_strength() const;
	const int get_dexterity() const;
	const int get_intelligence() const;
	const int get_speed() const;
	const double get_resource_mod() const;
	const gc::Affiliation get_affiliation() const { return affiliation; }
	const double get_status_mod(gc::StatusType st) const;
	const std::map<gc::StatusType, double> get_all_status_and_values() const;
	const bool get_position()  const { return is_front; }
	virtual const int get_hp() const;
	virtual const int get_temp_hp() const;
	virtual const int get_intial_temp_hp() const;
	virtual const int get_max_temp_hp() const;
	virtual const int get_max_hp() const;
	const virtual double get_turn_progress() const;
	const int get_accuracy() const;
	const int get_dodge() const;
	const int get_damage_reduction() const;
	const int get_effect_accuracy() const;
	const int get_effect_resist() const;
	const int get_armor_ignore() const;
	const int get_critical_chance() const;
	const double get_critical_damage_mod() const;

	virtual const std::string get_resource_display() const { return ""; }

	virtual Ability* get_ability(int i);

	int change_hp(int c); 
	void change_damage_taken(int c) { damage_taken += c; }
	void set_affiliation(gc::Affiliation a) { affiliation = a; }
	void add_status(Status *stat);
	virtual bool is_turn();
	void reset_increment() { turn_increment = 0; }
	
	virtual void update();
	virtual bool is_defeated();
protected:
	// Stats
	int level;
	int base_con,
		base_str,
		base_dex,
		base_int;
	double con_per_lvl,
		str_per_lvl,
		dex_per_lvl,
		int_per_lvl;
	int base_turn_cycle,
		base_accuracy,
		base_dodge,
		base_armor,
		base_effect_accuracy,
		base_effect_resist,
		base_hp;

	// Battle Stats
	int damage_taken;
	int temp_hp; 
	gc::Affiliation affiliation;
	bool is_front;
	int turn_increment;
	StatusList status_list;

	//Abilties
	std::vector<Ability *> abilities;

	//Misc
	std::string name;
};

class Party {
public:
	Party() {}
	Party(std::vector<GameUnit*> p, gc::Affiliation a);
	virtual ~Party() {}

	const std::vector<GameUnit *>& get_party_members() const { return party_members; }
	bool is_party_defeated();
	const gc::Affiliation get_party_affiliation() const { return affiliation; }
private:
	std::vector<GameUnit*> party_members;
	gc::Affiliation affiliation;
};


class PlayerUnit : public GameUnit {
public:
	PlayerUnit() : GameUnit() {}
	PlayerUnit(std::string n, int lvl) : GameUnit(n, lvl) {}
	virtual ~PlayerUnit();

	virtual void update();
	virtual void update_resource();
	//virtual int do_turn(BattlePhase *b);
protected:
};

//Non-Player Character
class Npc : public GameUnit {
public:
	Npc() {}
	Npc(std::string n) : GameUnit(n) {}
	Npc(int c, int s, int d, int i) : GameUnit(c, s, d, i) {}
	Npc(std::string n, int c, int s, int d, int i) : GameUnit(n, c, s, d, i) {}
	Npc(std::string n, int lvl) : GameUnit(n, lvl) {}
	virtual ~Npc();

	//virtual int do_turn(BattlePhase *b);
protected:
};

class Soldier : public PlayerUnit {
public:
	Soldier() : PlayerUnit() {}
	Soldier(std::string n, int lvl); 
	virtual ~Soldier() {}

	virtual Ability* get_ability(int i);

	virtual void assign_abilities();
	virtual void update_resource();
private:
	std::vector<WarriorAbility*> w_abilities;
};

class Mage : public PlayerUnit {
public:
	Mage() {}
	Mage(std::string n, int lvl); 
	virtual ~Mage() {}
	
	virtual Ability* get_ability(int i);

	virtual void assign_abilities();
	virtual void update_resource();
	virtual void change_mp(int change);

	virtual const int get_mp() const { return int(current_mp); }
	virtual const std::string get_resource_display() const { return std::to_string(get_mp()); }
private:
	std::vector<MageAbility*> m_abilities;

	double current_mp;
	double base_mp_regen;
};

class Thief : public PlayerUnit {
public:
	Thief() {}
	Thief(std::string n, int lvl); 
	virtual ~Thief() {}
	
	virtual Ability* get_ability(int i);

	virtual void assign_abilities();
	virtual void update_resource();
	virtual void change_combo_points(gc::ComboPoints cp);

	virtual const gc::ComboPoints& get_combo_points() const;
	virtual const std::string get_resource_display() const;

private:
	std::vector<RogueAbility*> t_abilities;

	gc::ComboPoints combo_points;
};