# GameQueue
This is a Parallel Processing program to simulate waiting in an online game queue

* This game plays for 5 epochs in which players in the game will fight for dominion over a continent
* Players will be divided into 2 teams taken from the "players.dat" file
* This "players.dat" file will contain a series of "A"s or "H"s to determine their allegiance
* This will be followed by a "D" and a number 0 through 4 to specify the dragon's starting continent
* There are 4 continents, each holding up to 5 players, for a max of 20 players in the game
* Players will be placed in the continents on a first-come-first-serve basis
* Players not in the starting game will be placed in a queue to wait for an opening
* Players on a continent will fight for control with the faction with the majority killing the first member of the minority team
* The dragon will then eat a player from the continent it is currently on
* The dragon will prefer to eat a player belonging to the "A" faction, but if none are present it will eat the first "H"
* Following this round, the surviving players will move to the next continent while the dragon will move to the previous one
* Players waiting in the queue will then enter the game at available positions
* The players killed in the previous round will go to the back of the queue and wait to enter in a future epoch
* All of these actions are done in parallel so it should occur simultaneously
