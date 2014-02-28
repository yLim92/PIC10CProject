#include <map>
#include <utility>

#include "utility.h"

class Floor;
class Event;
class ExplorePhase;
class Party;

class EventList {
public:
	EventList();
	virtual ~EventList() {}

	gc::RoomCode do_room_event(utility::coordinates u, Party *p);
	void populate_event_table(Floor* f);
	void set_possible_events();
private:
	std::map<utility::coordinates, Event*> event_table;
	vector<Event> possible_events;

	enum events{
		battle_event,
		money_event_a,
		money_event_b,
		money_event_c,
	};
};                                                                                                                                                              

class Event {
public:
	Event() : room_code(gc::RoomCode::unvisited) {}
	virtual ~Event();

	virtual gc::RoomCode do_event(Party *p);
protected:
	gc::RoomCode room_code;
};

class BattleEvent: public Event {
public:
	BattleEvent() : Event() {}
	virtual ~BattleEvent() {}

	virtual gc::RoomCode do_event(Party *p);

};

class MoneyEventA: public Event {
public:
	MoneyEventA() : Event() {}
	virtual ~MoneyEventA() {}

	virtual gc::RoomCode do_event(Party *p);
};

class MoneyEventB: public Event {
public:
	MoneyEventB() : Event() {}
	virtual ~MoneyEventB() {}

	virtual gc::RoomCode do_event(Party *p);
};

class MoneyEventC: public Event {
public:
	MoneyEventC() : Event() {}
	virtual ~MoneyEventC() {}

	virtual gc::RoomCode do_event(Party *p);
};