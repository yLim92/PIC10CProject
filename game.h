class ExplorePhase;

using namespace std;

class Game {
public:
	Game();
	~Game() {}

	void intro();
	void start();
	
	void new_game();
	void continue_game();
private:
	ExplorePhase* explorePhase;
};