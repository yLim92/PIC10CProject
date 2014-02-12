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

	gc::RoomCode do_room_event(utility::coordinates u);
	void populate_event_table(Floor* f, Party *p);
private:
	std::map<utility::coordinates, Event*> event_table;
};                                                                                                                                                              

class Event {
public:
	Event();
	Event(Party *p) : party(p), room_code(gc::RoomCode::unvisited) {}
	virtual ~Event();

	virtual gc::RoomCode do_event();
protected:
	gc::RoomCode room_code;
	Party *party;
};

class BattleEvent: public Event {
public:
	BattleEvent() {}
	BattleEvent(Party *p) : Event(p) {}
	virtual ~BattleEvent() {}

	virtual gc::RoomCode do_event();
protected:

};