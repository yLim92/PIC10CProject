/*
Yongjie Lim
UID: 703912832
Week 3 Submission

DIRECTIONS:
-PLEASE manually expand the console height for the map.  Otherwise the output might look REALLY strange.
-Select new game and select whatever difficulty
-Game will start off with a battle; finish the battle and the game should enter exploration phase for the rest of the time.
-Try out the floor navigation!
-Please forgive the flickering.  I am aware console applications aren't ideal for games.

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


int main() {
	set_window_size(100, 70);

	Game g = Game();
	g.start();

	return 0;
}

/* QUESTIONS

-Is it bad to return a non constant pointer for error checking purposes? (want to check for nullptr)
-protected vs private
-would it be wise to template my abilities?
-How best to hook up all the output to battleview? How about saving?
*/