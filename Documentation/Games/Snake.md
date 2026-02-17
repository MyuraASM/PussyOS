# Snake

Classic Snake game for PussyOS with AI autopilot mode.

## Starting the Game

```
snake
```

Game runs in fullscreen mode at 80x25 resolution.

## Controls

**Manual Mode:**
- UP arrow - Move up
- DOWN arrow - Move down
- LEFT arrow - Move left
- RIGHT arrow - Move right
- K - Toggle AI autopilot
- ESC - Exit game
- R - Restart (after game over)

**AI Mode:**
- K - Toggle back to manual control
- ESC - Exit game

## Gameplay

Eat red food blocks to grow longer. Don't hit walls or your own body. Score 10 points per food eaten.

**Starting State:**
- Snake length: 3 segments
- Initial direction: Moving right
- Snake spawns at center of screen

**Movement:**
- Cannot reverse direction (prevents instant death)
- Horizontal movement is faster than vertical (300k vs 600k ticks)
- Snake grows by 1 segment per food eaten
- Maximum length: 100 segments

**Game Over Conditions:**
- Hit outer walls
- Collide with own body

## AI Autopilot

Press K to enable AI mode. The snake will play itself using an aggressive pathfinding algorithm.

**AI Strategy:**
- Moves directly toward food whenever possible
- Prioritizes diagonal approach (horizontal then vertical)
- Only avoids obstacles at the last second
- Falls back to any safe direction if direct path blocked
- Never reverses direction
- Checks collision accounting for tail movement

**AI Collision Detection:**
The AI uses a smart collision system that accounts for whether the tail will move:

```c
// When eating food: tail stays, check all segments
// When moving normally: tail moves, skip last segment in collision check
```

This prevents false collision detection and allows tighter movement.

**AI Decision Priority:**
1. Move toward food (horizontal or vertical)
2. Try perpendicular directions
3. Try any remaining safe direction
4. Keep current direction if nothing safe

## Visual Elements

**Snake:**
- Head: Bright green (0x0A) in manual mode, cyan (0x0B) in AI mode
- Body: Dark green (0x02)
- Character: Full block (219)

**Food:**
- Color: Red (0x0C)
- Character: Full block (219)
- Spawns at random empty location

**Walls:**
- Color: Light gray (0x07)
- Forms border around play area

**Background:**
- Dark gray (0x08) inside play area

**UI Bar (bottom row):**
- Shows current score
- Shows controls based on mode
- White on gray (0x70)

**Game Over:**
- Red background message (0x4F)
- Displays at screen center

## Technical Implementation

**Data Structures:**

```c
typedef struct {
    int x;
    int y;
} Segment;

Segment snake[MAX_SNAKE_LEN];  // Up to 100 segments
```

**Game Constants:**
```c
GAME_WIDTH:      80
GAME_HEIGHT:     24
MAX_SNAKE_LEN:   100
```

**Timing:**
- Horizontal movement: 300,000 tick threshold
- Vertical movement: 600,000 tick threshold (slower)


**Random Number Generation:**

Uses linear congruential generator (LCG):
```c
rand_seed = rand_seed * 1103515245 + 12345;
return (rand_seed / 65536) % max;
```

Seed initialized with snake position for variation.

**Food Placement:**

Attempts up to 1000 times to find empty spot not on snake body. Checks each snake segment before placing.

**Collision Detection:**

Two-phase check:
1. Wall check: `x <= 0 || x >= WIDTH-1 || y <= 0 || y >= HEIGHT-1`
2. Body check: Compare against all snake segments

Special handling for growth:
- Normal move: Skip tail segment (it will move away)
- Eating food: Check all segments (tail doesn't move)

**Movement System:**

Snake moves by:
1. Adding new head at `(head.x + dir_x, head.y + dir_y)`
2. If not eating: Shift all segments forward, drop tail
3. If eating: Keep all segments, add new head, increment length

**Direction Input Buffering:**

Uses `next_dir_x` and `next_dir_y` to buffer next direction. Applied on next tick. Prevents missing inputs between ticks.

**Key Scancodes:**
- UP: 0x48
- DOWN: 0x50
- LEFT: 0x4B
- RIGHT: 0x4D
- K: 0x25
- ESC: 0x01
- R: 0x13





## Integration

Add to shell command handler:

```c
if (strcmp(command, "snake") == 0) {
    game_start();
}
```

Requires:
- VGA text buffer access (`struct Char* buffer`)
- Port I/O functions (inb)
- Print functions (print_clear, print_str, etc.)

## Customization

**Speed Adjustment:**
```c
// In game_start() loop:
if (dir_x != 0) {
    speed_threshold = 300000;  // Horizontal speed
} else {
    speed_threshold = 600000;  // Vertical speed
}
```

Lower values = faster game.

**Score Per Food:**
```c
score += 10;  // Change to adjust scoring
```

**Starting Length:**
```c
snake_len = 3;  // Initial snake length
```

**Colors:**
```c
buffer[idx].color = 0x0A;  // Snake head (manual)
buffer[idx].color = 0x0B;  // Snake head (AI)
buffer[idx].color = 0x02;  // Snake body
buffer[idx].color = 0x0C;  // Food
buffer[idx].color = 0x07;  // Walls
buffer[idx].color = 0x08;  // Background
```


## Known Behavior

**Speed Asymmetry:**
Vertical movement is intentionally slower (2x threshold). This compensates for VGA text mode's non-square character cells, making diagonal movement feel more natural.




**Food Spawn:**
In rare cases with full screen, may fail to find empty spot after 1000 attempts. Snake would be too long for playable area anyway.

## Exit Behavior

Game restores terminal state on exit:
- Clears screen
- Resets cursor position to (0,0)
- Shows shell prompt ">"
- Returns to normal shell operation

