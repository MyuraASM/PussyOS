# Pong

Classic two-player Pong game for PussyOS. First to 5 points wins.

## Starting the Game

```
pong
```

Game runs in fullscreen mode at 80x24 resolution.

## Controls

**Player 1 (Left paddle - Blue)**
- W - Move up
- S - Move down

**Player 2 (Right paddle - Red)**
- UP arrow - Move up
- DOWN arrow - Move down

**General**
- ESC - Exit game
- R - Restart (after game over)

## Gameplay

The ball starts at center and moves toward a random direction. Hit it with your paddle to send it back.

**Ball Physics:**
- Bounces off top and bottom walls
- Speeds up based on where it hits the paddle
- Hitting near paddle center = straight shot
- Hitting near edges = angled shot
- Ball has a visible trail effect

**Scoring:**
- Point scored when ball passes opponent's paddle
- First player to 5 points wins
- Ball resets to center after each point

## Visual Elements

**Paddles:**
- Left paddle: Blue (0x09)
- Right paddle: Red (0x0C)
- Height: 5 characters
- Fixed X positions near screen edges

**Ball:**
- White block (0x0F)
- Fading trail effect (5 positions)
- Trail colors fade from white to dark gray

**Screen:**
- Black background (0x00)
- Dashed center line (0x08)
- Bottom status bar (0x70) shows scores and controls

**Game Over:**
- Winner message displays in center
- Blue background for Player 1 win (0x9F)
- Red background for Player 2 win (0xCF)

## Technical Details

**Screen Layout:**
- Game area: 80 columns x 24 rows
- Bottom row (25): Status bar with scores and controls

**Timing:**
- Paddle movement: Updates every 100,000 ticks
- Ball movement: Updates every 300,000 ticks
- Render updates: Every 100,000 ticks

**Constants:**
```c
SCREEN_WIDTH:   80
SCREEN_HEIGHT:  24
PADDLE_HEIGHT:  5
PADDLE_X1:      2   (left paddle)
PADDLE_X2:      77  (right paddle)
TRAIL_LENGTH:   5
```

**Ball Behavior:**
- Initial direction: (1, 1) - moves right and down
- After scoring:
  - Player 1 scores: Ball goes left (-1, -1)
  - Player 2 scores: Ball goes right (1, 1)
- Wall bounce: Inverts Y direction
- Paddle collision: Inverts X direction, adjusts Y based on hit position

**Collision Detection:**
- Ball checks if it's within 1 character of paddle X position
- Y position must be within paddle's 5-character height
- On hit, ball rebounds and trail resets

## Game States

**Playing:**
- Both paddles respond to input
- Ball moves and bounces
- Scores update on goals

**Game Over:**
- Triggered when either player reaches 5 points
- Paddles frozen
- Winner message displayed
- Only ESC and R keys active

## Exit Behavior

Game restores terminal state on exit:
- Clears screen
- Resets cursor position
- Shows shell prompt ">"
- Returns to normal shell operation

## Code Structure

**Main Functions:**
```c
pong_start()      // Entry point, main game loop
pong_init()       // Initialize game state
pong_render()     // Draw all game elements
update_ball()     // Ball physics and collision
reset_trail()     // Reset ball trail positions
```

**Input Handling:**
- Checks keyboard port (0x64) for data
- Reads scancode from port (0x60)
- Tracks key press/release states
- Supports simultaneous key presses (both players can move at once)

**Key Scancodes:**
- W: 0x11
- S: 0x1F
- UP: 0x48
- DOWN: 0x50
- ESC: 0x01
- R: 0x13

## Integration

Add to shell command handler:

```c
if (strcmp(command, "pong") == 0) {
    pong_start();
}
```

Requires:
- VGA text buffer access (`struct Char* buffer`)
- Port I/O functions (inb)
- Print functions (print_clear, print_str, etc.)

## Customization

Adjustable parameters in code:

**Speed:**
```c
paddle_move_counter threshold: 100000  // Lower = faster paddles
ball_speed_counter threshold:  300000  // Lower = faster ball
```

**Scoring:**
```c
if (score1 >= 5) game_over = 1;  // Change 5 to different win condition
```

**Paddle Size:**
```c
#define PADDLE_HEIGHT 5  // Change paddle height
```

**Colors:**
```c
buffer[...].color = 0x09;  // Blue paddle
buffer[...].color = 0x0C;  // Red paddle
buffer[...].color = 0x0F;  // White ball
```

VGA color format: 0xBF (B=background 0-7, F=foreground 0-15)

## Known Behavior

- Ball can move diagonally when hitting paddle edges
- Maximum Y velocity is ±2 (paddle height / 2)
- No AI opponent (requires two players)
- Ball doesn't accelerate over time
- Game doesn't save high scores
- Trail resets on any collision to prevent visual artifacts
