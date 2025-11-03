# COMP3016-CW1-Immersive-Game-Technologies-


## Gameplay Description
This game is a rogue-like game where the player character moves through randomly generated levels, filled with several boxes that increase in number with each level, and the player must reach the exit to each level while battling various enemies that get stronger. Whenever the player encounters an enemy, they will initiate turn-based combat, where the player and enemy will alternate attacks. After defeating enemies, exiting the level, and finding some on the floor, the player will be rewarded with gold to upgrade their stats.  
  
## Dependencies Used
There are no additional dependencies used in this code. It only uses the standard C++ library. There are 12 includsions that are required for the code to function and that's it.  
  
## Use of AI
<img width="934" height="755" alt="image" src="https://github.com/user-attachments/assets/6e4609ce-340e-44c4-9bd4-b4818aed279c" />
  
## Game Programming Patterns
The biggest programming pattern that is being used is the double buffer. At the beginning of the level, it has to draw many individual items onto the screen, the most important ones being the text at the top of the screen, the main box that everything else is in, and the room containing the player. Every time the player moves, we will check to see if the player is in a corridor or room that has not been revealed and draw the newly revealed area and any enemies or exits that might be inside.  
The entire game runs on a game loop. The loop in main checks for any inputs, then processes them and runs any other relevant code based on those inputs and draws any new items appearing. There are multiple loops in the creation of rooms, firstly to place a room down in a random location for each room up to the maximum allowed for that level and then there is a loop to try and link two rooms together with a corridor. Additionally, there is a game loop for combat, as it is a turn-based system.  

  
## Game Mechanics
The primary mechanics of this game are its dungeon exploration and the turn based combat. The dungeon starts of fairly small in the first level and grows larger with each level, both in the size of the whole window and in the maximum number of rooms that can appear. Each room will be randomly placed somewhere in the window and each will be connected to at least one other room via corridors. After each rooms has been placed, the player is placed into the center of a random room and then the same is done for the exit. Then, any rooms that only have one corridor (aka dead ends) will have a piece of gold placed in the center and then any other rooms in the level that are empty will have an enemy placed in them. Each room and corridor will be hidden at the start of a level until a player walks into that area. This is to stop the player from walking directly to the level exit and make them more inclined to explore the entire level.  
The player character and enemy each have three stats used for combat: health, defence, and strength. When either party attacks the other, the attacker will deal damage equal to their strength and subtract it from the defender's health. The player has a choice to guard instead of attacking, and the enemies have a chance to do the same. When anyone guards, until their next turn, the next instance of damage they take will be reduced by their defence. This cannot reduce the damage below 1. If a player or enemy is hit while guarding, they will initiate a parry, dealing damage to the attacker. This damage is inversely propotional to the defence stat to balance this mechanic and to avoid making the strength stat redundant.  
At the end of each level, the player has the ability to upgrade any of their stats using the gold they have obtained by playing, or purchase a potion to restore health either while exploring or in combat. Each stat initially costs one gold to upgrade the stat and will increment for each time you have previously upgraded that stat during the game .To increase the difficulty of the game, the enemies will also gain increases to their stats at the end each level. The enemy will have three "upgrade points" to distribute among the three stats, with each point increasing that stat by one or two, depending on the stat. By default, each stat has an equal chance of being upgraded, but to make the enemies more suited to the player, each stat gains a higher chance of being upgraded depending on how many times the player has upgraded a corresponding stat during that round of upgrades. For example, the player levels up their strength twice and their health once at the end of level 4. The enemy will be 25% more likely to upgrade strength and 50% more likely to upgrade defence, and this will reset at the end of level 5.  

## Sample Screens
<img width="1324" height="712" alt="image" src="https://github.com/user-attachments/assets/0d94a435-311a-43c3-9c3f-3a8c15a8bcf3" />  

#### This is what the average level will look like later on in the game, once the player has explored everywhere.  

<img width="814" height="454" alt="image" src="https://github.com/user-attachments/assets/c52f7ea4-fb10-4f4c-8afc-a30c0fd4121a" />  

#### When a player walks into a room, the entire room will be revealed. When in an unexplored corridor, only the spaces adjacent to the player will be revealed instead of the entire corridor.  

<img width="1465" height="696" alt="image" src="https://github.com/user-attachments/assets/3160a5d3-5b26-486c-b6af-f1a4e85635a5" />  

#### When a stat is upgraded, the gold, players stats, and upgrade cost will update accordingly. Health upgrades increase the health by 2 because it is probably the most important stat and healing is limited. This is also why potions are only two gold and do not increase in price, because they are the only source of healing in the game.  

<img width="1444" height="490" alt="image" src="https://github.com/user-attachments/assets/53eaf7c7-ffd2-4b38-88d3-a7ec03d38cf6" />  

#### In addition to the three actions listed previously in the markdown file, the player has the option of running away. This has a 40% chance of ending the battle and returning the player to their location prior to the battle. As an additional note, using a potion heals the player for an amount equal to half of their maximum health. Potions can only be used if there is one in the player's inventory and cannot be used if their health is full.  

<img width="1459" height="297" alt="image" src="https://github.com/user-attachments/assets/5dece2b3-beb9-4f01-8cc7-ef0f37bcaab1" />  

#### Once either the player or the enemy have no health remaining, combat will end. If the enemy has no health, the player has won combat, gains one to three gold and can continue exploring the level while the enemy disappears. If the player has no remaining health, then they have been defeated and the game will end.  

## Brief Evaluation  
During this project, I have successfully created a fully functioning rogue-like game with a well designed combat and levelling system. Its aesthietic was deigned around challenge and I think that I did a good job in making the game challenging enough to engage the player without it being possible. I am particularly proud of the way I used the defence stat so that it was a useful stat that didn't make any other stats useless.
