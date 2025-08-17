# Game Design Document

## Section 0: Summary
Title: Aim.exe
Team Members: Kyle Berkson, Kenny Chan, Alvin Zhang
Concept statement: A 2D physics-based competitive shooting game where players take turns launching projectiles at opponents


## Section 1: Gameplay
Each game begins with the player selecting their character and arena (Forest Bluff, Windy Mesa, or Moon), each with distinct gravity and terrain profiles. The player alternates turns with the computer, with a 45-second timer per turn. On your turn, you click and drag with the mouse to set your shot angle and power, guided by an arrow showing initial trajectory. Releasing the mouse fires the arrow. After each shot, the game uses a finite state machine to rotate to the next player and pans the camera smoothly to their position. Arrows are subject to gravity, wind, and drag. Each archer starts with a health bar, which decreases when hit. The game ends when a player’s HP reaches 0. 

**Summary**:
Controls: Mouse (click & drag to aim and set shot strength, release to shoot), z to zoom out. 
Physics: The collision library will be used to determine when an arrow has collided with a player or the ground. We will use the create_newtonian_force, and body_apply_force to adjust the trajectory of the arrow as it travels through the air. 
Game flow: (from main menu) select character/arena -> play game -> win/lose -> back to main menu 
Graphics: Each character will have custom sprites and there’ll be distinct images for different arenas. Graphics library will also be used to create vector level terrain, projectiles, particle effects, HUD (health, turn timer).


## Section 2: Feature Set

Priority 1 features:
- Turn engine: determine which character should be shooting (ai or player), give that player ability to shoot or not.
- Shooting system: Mouse drag sets angle and power, with an arrow starting at the position of the character it is being shot from showing direction and power via angle and arrow length.
- Bullet creation, collision handling, and core HUD: Create bullets and collisions– on hit, subtract HP proportionally to the arrow’s speed. Health display, 45s turn timer.  

Priority 2 features:
- Basic CPU implementation: AI samples random angle/power, simulates trajectory, picks closest shot. 
- Damage handling / game end handling: update health accordingly and logic for whether the game is ended. 
- Game start menu: Play button and controls button -> Three selectable arenas: forest arena, windy mesa, low-g moon: unique terrain vertices, gravity constant, sky color

Priority 3 features: 
- Crates located on the map take projectile damage; on break the 0 HP
- Arrow emits fading particle trail; terrain hit spawns dust burst along surface normal. 
- Wind system: wind vector randomizes every turn, affecting the trajectory of arrows once shot. 

Priority 4 features:
- Camera pan: camera focuses on active shooter, not showing the entire screen. Player has the option to momentarily zoom the camera out by clicking a key. When the arrow is shot, the camera fully zooms out.
- Game end menu: look for game ending (HP of player = 0), display you win/lose, button to redirect back to game start menu. 
- Arrow variants: Players can shoot a normal arrow, heavy arrow (decreased range, more weight -> more damage), or multi shot (3 arrows spawn from shot location w/ +- 7 deg, each has ⅓ weight of a normal arrow) 


## Section 3: Timeline
Each group member will be assigned a feature as follows. We will collectively implement higher priority features first, especially those which serve as a base for other features:  

Kyle 
- 1a: Turn engine (week 1) 
- 2a: Basic CPU implementation(week 1)
- 3c: Wind system (week 2) 
- 4a: Camera pan (week 2) 

Kenny
- 1c: Bullet creation, collision handling and core HUD (week 1)
- 2c: Damage handling / game end handling (week 1) 
- 3b: Particle trails / bursts (week 2)
- 4c: Game end menu (week 2)

Alvin
- 1b: Shooting system (week 1)
- 2b: Game start menu (week 1)
- 3a: Destructible crates (week 2)
- 4b: Arrow variants(week 2)



## Section 4: Disaster Recovery
**Kyle**: If fallen behind, go to office hours to first flesh out any technical details. This group member will then meet with Kenny and Alvin in order to let them know of the things they are behind on and to establish a working, yet feasible timeline for getting back on track. This timeline must be prioritized and should result in getting back on track as soon as possible. 

**Kenny**: In the scenario that this group member falls behind, he will establish a daily schedule to begin prioritizing code-related tasks. Additionally, he will use Kyle and Alvin for conceptual feedback in order to iterate more efficiently. As an alternative, this group member will attend office hours to address any further details that remain unresolved. 

**Alvin**: He will identify what the cause of the slowdown is. If it’s a technical issue, then he’ll meet with Kenny and Kyle to see if we can solve it together, and if not, then go to office hours to figure it out. Regardless, he’ll dedicate more time to the given task to catch up, prioritizing it over sleep if necessary.

**<ins>All</ins>**: If necessary, we’ll redistribute tasks to assist the delayed member until needed features are completed. This will mainly take the case if a feature that the delayed member is necessary for another remember to move on. If a slowdown is caused by a task being difficult or something not working, group members should convene and/or use pair programming techniques where possible. Also, to ensure seamless integration as these features come together, we’ll do code reviews as needed. 