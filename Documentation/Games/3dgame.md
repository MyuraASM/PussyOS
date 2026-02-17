# 3D Shooter

Wave-based first-person shooter with raycasting graphics. Survive enemy waves and rack up points with kill multipliers.

## Starting the Game

```
3dgame
```

Runs fullscreen at 80x25 text mode resolution with raycasting renderer.

## Controls

- W - Move forward
- S - Move backward
- A - Strafe left
- D - Strafe right
- LEFT arrow - Turn left
- RIGHT arrow - Turn right
- SPACE - Shoot
- ESC - Exit game
- R - Restart (after death)

## Gameplay

**Objective:** Clear waves by killing all enemies. Survive as long as possible.

**Health System:**
- Start with 5 HP
- Lose 1 HP on enemy contact (once per second)
- Collect health pickups to restore HP (max 5)
- Game over at 0 HP

**Scoring:**
- 100 points per enemy kill
- 2x multiplier for quick consecutive kills (within 300 ticks)
- Multiplier shown as [2X] in green when active
- Multiplier resets after 300 ticks without kills

**Wave Progression:**
- Wave 1: 3 zombies, 0 ghosts
- Each wave adds 1 more zombie
- Ghosts appear starting wave 3 (1 per 2 waves)
- Health pickups spawn: 1 per wave, 2 every 3rd wave
- Enemies spawn far from player (min distance: 3 tiles)

## Enemy Types

**Red Zombies:**
- Slow movement
- 1 Hp
- Patrol randomly when player is far
- Chase player within 5 tile radius
- Cannot phase through walls

**Blue Ghosts:**
- Fast movement
- 2 HP (requires 2 shots to kill)
- Erratic movement pattern
- 10% chance to phase through walls
- More aggressive than zombies

Both enemy types deal 1 damage on contact (rate-limited to once per second).

## Map

16x16 tile grid with colored wall types and open spaces. Outer walls form boundary. Interior features various colored sections for visual variety.

## Visual Elements

**Raycasting Graphics:**
- Textured walls with distance-based shading
- Darker walls at distance
- Gradient ceiling (blue to dark)
- Gradient floor (dark gray patterns)
- Wall shading varies by hit side

**Sprites:**
- Enemies: Full block characters with depth sorting
- Zombies: Red (0x0C)
- Ghosts: Blue (0x09)
- Health pickups: Green cross (0x0A)
- Top half bright, bottom half dark for depth

**HUD (bottom row):**
- HP: Red hearts (♥)
- Wave: Current wave number
- Score: Yellow text
- Multiplier: Green [2X] indicator when active
- Game over message: Red background

## Game Mechanics

**Shooting:**
- Raycasts forward from player position
- Hits first enemy in line of sight
- No bullet drop or travel time
- Instant hit detection
- Can't shoot through walls

**Movement:**
- Forward/backward follows view direction
- Strafing moves perpendicular to view
- Collision detection prevents walking through walls
- Slide along walls if diagonal movement blocked

**Enemy AI:**
- Chase player when within 5 tile radius
- Move toward player using direct pathfinding
- Zombies patrol randomly when player is far
- Ghosts move erratically with random jumps
- Both types avoid walls (ghosts can phase through occasionally)
- Contact damage rate-limited to prevent instant death

**Wave System:**
- New wave spawns when all enemies killed
- Enemy count scales with wave number
- Health pickups placed at random empty locations
- Spawns avoid player starting area

## Technical Details

**Rendering:**
- Fixed-point math (16.16 format) for precision
- Field of view: 60 degrees
- Raycasting with DDA algorithm
- Sprite depth sorting
- Distance-based color attenuation

**Collision:**
- Grid-based tile collision
- Separate checks for X and Y axes
- Slide along walls if diagonal blocked

**Timing:**
- Game ticks increment each frame
- Used for multiplier timeout
- Enemy damage rate limiting
- Patrol timer for zombies

**Constants:**
```
MAP_WIDTH:      16
MAP_HEIGHT:     16
MAX_SPRITES:    32
TEXT_COLS:      80
TEXT_ROWS:      25
```



## Integration

Add to shell:

```c
if (strcmp(command, "3dshooter") == 0) {
    walking_simulator_start();
}
```

Requires VGA text buffer access and port I/O.






