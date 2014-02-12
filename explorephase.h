#include<vector>
#include "events.h"

enum Difficulty;

class Floor;
class EventList;
class Party;

class ExplorePhase {
public:
	ExplorePhase();
	ExplorePhase(Difficulty d, Party* p);
	~ExplorePhase() {}

	void explore();
	void post_event_phase();
	void add_new_floor();

private:
	std::vector<Floor *> floors;
	Floor* current_floor;
	Difficulty difficulty;
	EventList event_list;
	Party* party;
};