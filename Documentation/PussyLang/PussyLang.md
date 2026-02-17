# PussyLang

A simple scripting language for PussyOS. Scripts run in an interactive fullscreen mode and output to both terminal and file.

## File Extensions

`.p` or `.pussy`

## Running Scripts

```
pussy filename.p
```

Scripts execute in interactive mode with a scrollable output viewer. Press ESC to exit.

## Variables

Variables start with `$`. They're created automatically on first use.

```
set $name "value"
set $count 10
```

Access variables with `$name` in any command. They expand inline.

```
say "Hello $name"
```

## Output

**say** - Print text with newline
```
say "Hello World"
say $variable
```

**sayinline** - Print text without newline
```
sayinline "Loading"
sayinline "..."
```

**outfile** - Set output filename (default: output.txt)
```
outfile "results.txt"
```

All output goes to both terminal and the specified file.

## Arithmetic

**add** - Add to variable
```
set $x 5
add $x 3
# $x is now 8
```

**sub** - Subtract from variable
```
sub $x 2
# $x is now 6
```

**bitand** - Bitwise AND, result in $result
```
bitand $a $b
say $result
```

## Control Flow

**if** - Conditional execution
```
if $x == 10 {
    say "x is 10"
}

if $x < 5 {
    say "x is less than 5"
}
```

Operators: `==` `!=` `<` `>` `<=` `>=`

**loop** - Repeat N times
```
loop 5 {
    say "Hello"
}

loop $count {
    say $x
    add $x 1
}
```

**while** - Loop with condition
```
set $i 0
while $i < 10 {
    say $i
    add $i 1
}
```

Max 10,000 iterations to prevent infinite loops.

## Lists

**list** - Create empty list
```
list $mylist
```

**push** - Add element
```
push $mylist "item1"
push $mylist $variable
```

**get** - Get element at index, result in $result
```
get $mylist 0
say $result
```

**setat** - Set element at index
```
setat $mylist 0 "new value"
```

**remove** - Remove element at index
```
remove $mylist 2
```

**pop** - Remove last element
```
pop $mylist
```

**size** - Get list size, result in $result
```
size $mylist
say $result
```

**clear** - Remove all elements
```
clear $mylist
```

Lists print as: `[item1, item2, item3]`

## Functions

**func** - Define function
```
func greet $name {
    say "Hello $name"
}

func add_numbers $a $b {
    add $a $b
    return $a
}
```

**call** - Call function
```
call greet "World"
call add_numbers 5 3
say $ret
```

Return values go to `$ret`. Functions have their own variable scope.

## User Input

**input** - Get user input (interactive mode only)
```
input $name
say "You entered: $name"
```

**getkey** - Get last key scancode without blocking
```
getkey $key
if $key != 0 {
    say "Key pressed: $key"
}
```

**waitkey** - Wait for key press
```
waitkey $key
say "You pressed: $key"
```

Common scancodes: ESC=1, Enter=28, Space=57, Up=72, Down=80, Left=75, Right=77

## Graphics

**clearscreen** - Clear screen
```
clearscreen
clearscreen 0x00
```

**pixel** - Draw colored block at coordinates
```
pixel 10 5 0x0C
# x y color
```

**pixelchar** - Draw character at coordinates
```
pixelchar 10 5 65 0x0F
# x y ascii_code color
```

**rect** - Draw filled rectangle
```
rect 10 5 20 10 0x0E
# x y width height color
```

**line** - Draw line (horizontal or vertical only)
```
line 0 10 79 10 0x0F
# x1 y1 x2 y2 color
```

Screen size: 80x25

Color format: 0xBF (B=background 0-7, F=foreground 0-15)

Colors go from 1 to 15



## System Commands

**cmd** - Execute OS command
```
cmd "ls"
cmd "cat file.txt"
```

**sleep** - Pause execution in milliseconds
```
sleep 1000
```

## Comments

Lines starting with `#` are ignored.

```
# This is a comment
set $x 5  # Comments can go here too
```

## Example Programs

**Hello World**
```
say "Hello World"
```

**Counter**
```
set $i 0
while $i < 10 {
    say $i
    add $i 1
}
```

**List Demo**
```
list $numbers
push $numbers "one"
push $numbers "two"
push $numbers "three"

size $numbers
say "List size: $result"

get $numbers 1
say "Second item: $result"

say $numbers
```

**Simple Animation**
```
clearscreen 0x00
set $x 0
loop 20 {
    pixel $x 12 0x0E
    sleep 50
    pixel $x 12 0x00
    add $x 1
}
```

**Input Example**
```
say "What's your name?"
input $name
say "Hello $name!"
```

**Function Example**
```
func fibonacci $n {
    if $n <= 1 {
        return $n
    }
    
    sub $n 1
    call fibonacci $n
    set $a $ret
    
    sub $n 1
    call fibonacci $n
    set $b $ret
    
    add $a $b
    return $a
}

call fibonacci 10
say "Fibonacci(10) = $ret"
```

## Interactive Mode

Scripts run fullscreen with:

- Output display area (22 lines)
- Status bar showing mode
- Help bar with controls

**Controls during execution:**
- UP/DOWN - Scroll through output
- ESC - Exit script

**Auto-scroll:** Enabled by default. Disables when you manually scroll up. Re-enables when scrolling to bottom.

## Output Files

All `say` and `sayinline` output is written to the output file (default: output.txt).

```
outfile "mydata.txt"
say "This goes to mydata.txt"
```

Files are saved in the filesystem after script completes.

## Special Variables

- `$ret` - Function return value
- `$result` - Result from get, size, bitand operations

## Limitations

- Max 50 variables
- Max 100 list items per list
- Max 20 functions
- Max 50 call depth
- Max 256 chars per variable value
- Max 100 lines per script
- Max 10,000 while loop iterations
- Output buffer: 128KB

## Syntax Notes

Strings can use quotes or not:
```
set $x "hello"
set $x hello
```

Both work. Quotes required for strings with spaces.

Variable expansion works in strings:
```
set $name "World"
say "Hello $name"
```

Blocks use `{` and `}`:
```
if $x == 5 {
    say "five"
}
```

Commands are case-sensitive and must be lowercase.
