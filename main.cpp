/*
Yongjie Lim
UID: 703912832
Week 5 Submission

*** IF BY SOME CHANCE ONE OF THE 19 FILES IS MISSING, CAN BE DOWNLOADED FROM THE REPOSITORY ON GIT:  https://github.com/yLim92/PIC10CProject IF IT'S NOT UPDATED, IT'S MY FAULT ***

DIRECTIONS:
-PLEASE manually expand the console height for the map.  Otherwise the output might look REALLY strange.
-Select new game and select whatever difficulty
-Game will start off with a battle; finish the battle and the game should enter exploration phase for the rest of the time.
-Try out the floor navigation!
-Please forgive the flickering.  I am aware console applications aren't ideal for games.

WEEK 5
Battling the linked statuses again.  Ridiculously tricky because:
	-Statuses can have a one-to-one, one-to-many, or many-to-many relationship, some reciprocal, others one way
	-Need to have a linked logic that is determined by the ability
	-Statuses must be dynamically created
	-Statuses have different targets and accuracies 
Other than that, completed most of the abilities of the Soldier and Mage class.  Based on time constraints, I will delay doing the rogue class for now,
and move on to creating enemies.
Introduced the concept of sustained abilities: abilities that last for as long as a condition is held.  In the mage case, Enfeeble and Defile require a resource cost every second. If the cost
cannot be paid, all sustained abilities disappear.

NEXT WEEK:
-Create a few enemies
-Begin populating the floor with events!!

WEEK 4
I realized that linked status might be more complex than one to one; that took a bit of time to resolve for the potential many-to-many relationships
The base classes have been set up along with their resource systems.  
	-Soldiers use cooldowns
	-Mages use magic points
	-Thiefs use combo points
I resolved the dynamic casting problem by using a getter that returns a base class ability pointer that is virtually redefined by the base classes.
Set up the calculations for all the things that might happen in battle, like dodge, accuracy, damage reduction, perfect hits, etc
Also, after viewing how bad the flicker makes the program look on slow computers, I might have to consider porting the project to Qt and using openGL or something
to make things look beautiful

NEXT WEEK:
-FINISH doing the abilities for the base classes.
-Begin creation of enemy npc classes

WEEK 3
The view took a lot longer than anticipated.
I've just begun to write the Soldier class. The past couple of days were spent messing around with multiple inheritance and refining the status system.
The issue of linked statuses came up.  Essentially, a status might have a dependent status that must be deleted with it...which causes some problems.
The Soldier class is COOLDOWN based.  The two abilities (morale boost & restrain) have a 10s and 20s cd, respectively, which applies after use.
Because I chose to do unique resource systems per class, dynamic casting will be necessary to update resource costs on abilities.

NEXT WEEK:
-Continue writing the 3 basic classes and enemies.
-Continue to refine the battle system; add dodge, accuracy, criticals, etc.

WEEK 2
I decided that extraneous things like shop menus and game set up should be done later; better to get a basic system down and build around it.  Shop menus will probably be pushed to week 7/8
This week I focused on setting up the dungeon explore and battle system.
There is text, but that will be implemented in a battle dialogue box in the BattleView class.  The display will be completed next week; for now it only shows hp and the timer bar
I took a useful function from the web for clearing the screen on windows OS; I may rewrite or just continue to use it.
Currently, the two abilities are attack and chargestrike.  Attack is just a basic damage dealer; charge strike is a 1.5 delayed attack.

NEXT WEEK:
-Begin writing the 3 basic classes and enemies.
-Devise a system for status effects
-Continue to refine the battle system; add dodge, accuracy, criticals, etc.

*/


#include "layout.h"
#include <iostream>
#include "game.h"
#include "utility.h"

#include <iomanip> 

void set_window_size(int w, int h)
{
	COORD coord = {w, h};
	SMALL_RECT Rect = {0, 0, w-1, h-1 };
	HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);

	if (!SetConsoleScreenBufferSize(Handle, coord))
		std::cout << "Failed to set buffer size\n" << std::endl;
	if (!SetConsoleWindowInfo(Handle, TRUE, &Rect))
		std::cout << "Failed to set window size\n" << std::endl;

}


int main() {/*
	vector<int> tf;
	int sz = 12;
	for (int q = 0; q < 10000; ++q) {
		vector<int> nums;
		for (int i = 0; i < sz; ++i){
			nums.push_back(utility::rng(100));
		}
		for (int i = 0; i < sz - 1; ++i){
			bool found = false;
			for (int j = i+1; j < sz; ++j){
				if (nums[i] == nums[j]){
					tf.push_back(1);
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
	}
	cout << "FOUND: " << tf.size() << endl;
	*/

	set_window_size(100, 70);

	Game g = Game();
	g.start();

	return 0;
}

/* 

QUESTIONS:
-Bad to have all my stuff clumped in this one file??
-breaking conditions?
-How to best do statuses? By Enum? Or by classes?

IDEAS:
-Ability based on turn bar, status effects


*/