# Graph

Interactive function plotter with pan, zoom, and custom expressions.

## Starting

```
graph
```

Runs fullscreen in text mode with 80x50 pixel resolution (half-block characters).

## Controls

**Function Selection:**
- 1 - Sine wave (y = sin(x))
- 2 - Cosine wave (y = cos(x))
- 3 - Parabola (y = x²/5)
- 4 - Cubic (y = x³/25)
- 5 - Circle (x² + y² = 25)
- 6 - Custom function mode

**View Controls:**
- Arrow keys - Pan view (up/down/left/right)
- +/= - Zoom in
- \- - Zoom out
- R - Reset view to default

**Cursor:**
- C - Toggle cursor on/off
- W/A/S/D - Move cursor (shows coordinates)

**Custom Functions:**
- F - Open function editor
- [ ] - Navigate between saved functions (in mode 6)

**General:**
- ESC - Exit to shell

## Built-in Functions

**y = sin(x)**
Standard sine wave. Yellow color.

**y = cos(x)**
Standard cosine wave. Cyan color.

**y = x²/5**
Parabola opening upward. Green color.

**y = x³/25**
Cubic curve. Magenta color.

**x² + y² = 25**
Circle with radius 5. Red color. Plots both top and bottom halves.

## Custom Functions

Press F to open the function editor.

**Supported operators:**
- `+` `-` `*` `/` - Basic arithmetic
- `^` - Power (exponents up to 5)
- `()` - Parentheses for grouping

**Built-in functions:**
- `sin(x)` - Sine
- `cos(x)` - Cosine
- `sqrt(x)` - Square root
- `abs(x)` - Absolute value

**Variable:**
- `x` - Independent variable

**Examples:**
```
x^2
sin(x)
x/2+3
sqrt(x)
2*x^3-x
cos(x)*sin(x)
(x-2)^2+1
abs(x)/x
x^2-4*x+3
```

**Editor:**
- Type expression at "y = " prompt
- Enter - Save function
- Backspace - Delete character
- ESC - Cancel without saving

**Storage:**
- Up to 5 custom functions can be saved
- Functions named f1, f2, f3, etc.
- Each gets unique color (blue variants)
- Navigate with [ ] keys in mode 6

## Display Features

**Grid:**
- Dotted grid lines at integer coordinates
- Dark gray color
- Main axes (x=0, y=0) in brighter gray

**Axes:**
- X-axis: Horizontal through y=0
- Y-axis: Vertical through x=0
- Origin at screen center (default view)

**Coordinates:**
- Displayed in bottom-right corner
- Yellow text
- Shows cursor position in world coordinates
- Updates as cursor moves
- Format: X:value Y:value (2 decimal places)

**Cursor:**
- Crosshair style
- Bright white color
- 5 pixel span (horizontal and vertical)
- Toggle with C key

**Function Label:**
- Top-left corner
- Shows current function equation
- Matches graph color

## View System

**Default View:**
- X range: -10 to +10
- Y range: -10 to +10
- Origin at screen center
- Scale: 1 unit per 4 pixels

**Panning:**
- Arrow keys move view in 10% increments
- View follows arrow direction
- Works while displaying any function

**Zooming:**
- Plus/equals: Zoom in (5/6 scale)
- Minus: Zoom out (6/5 scale)
- Both axes zoom together
- View center stays at current position

**Reset:**
- R key returns to default view
- Resets scale to 1:1
- Centers origin
- Resets cursor to center

## Expression Parsing

**Order of operations:**
1. Parentheses
2. Functions (sin, cos, sqrt, abs)
3. Power (^)
4. Multiply/divide (*, /)
5. Add/subtract (+, -)

**Number format:**
- Integers: `5`, `-3`
- Decimals: `3.14`, `0.5`
- Up to 4 decimal places precision

**Limitations:**
- Powers limited to exponents 0-5
- No nested functions beyond 2 levels
- Expression max length: 64 characters
- Negative results handled correctly
- Division by zero returns 0 (no crash)

## Colors

**Functions:**
- Sine: Yellow (0x0E)
- Cosine: Cyan (0x0B)
- Parabola: Green (0x0A)
- Cubic: Magenta (0x0D)
- Circle: Red (0x0C)
- Custom: Blue variants (0x09+)

**UI:**
- Grid: Dark gray (0x08)
- Axes: Light gray (0x07)
- Labels: White/yellow
- Cursor: Bright white (0x0F)
- Coordinates: Yellow (0x0E)



## Integration

Add to shell:

```c
if (strcmp(command, "graph") == 0) {
    graphics_math_start();
}
```

Requires VGA text buffer and keyboard port access.
