# Math

Command-line calculator for integer arithmetic and bitwise operations.

## Usage

```
math <operation> <num1> [num2]
```

All operations use 32-bit signed integers. Results print to terminal.

## Arithmetic Operations

**add** - Addition
```
math add 5 3
→ 8
```

**sub** - Subtraction
```
math sub 10 7
→ 3
```

**mul** - Multiplication
```
math mul 6 7
→ 42
```

**div** - Integer division (truncates toward zero)
```
math div 10 3
→ 3
```

**mod** - Modulo (remainder)
```
math mod 10 3
→ 1
```

**pow** - Power (exponent 0-31 only)
```
math pow 2 10
→ 1024
```

## Bitwise Operations

**and** - Bitwise AND
```
math and 12 10
→ 8
```

**or** - Bitwise OR
```
math or 12 10
→ 14
```

**xor** - Bitwise XOR
```
math xor 12 10
→ 6
```

**not** - Bitwise NOT (unary)
```
math not 5
→ -6
```

**shl** - Shift left (amount 0-31)
```
math shl 3 2
→ 12
```

**shr** - Shift right (amount 0-31)
```
math shr 12 2
→ 3
```

## Number Theory

**gcd** - Greatest common divisor
```
math gcd 12 18
→ 6
```

**lcm** - Least common multiple
```
math lcm 12 18
→ 36
```

**sqrt** - Integer square root (floor)
```
math sqrt 17
→ 4
```

**log2** - Floor of log base 2
```
math log2 16
→ 4
```

**fact** - Factorial (n ≤ 12)
```
math fact 5
→ 120
```

**fib** - Fibonacci number (n ≤ 46)
```
math fib 10
→ 55
```

## Comparison Operations

Return 1 for true, 0 for false.

**eq** - Equal to
```
math eq 5 5
→ 1
```

**ne** - Not equal to
```
math ne 5 3
→ 1
```

**lt** - Less than
```
math lt 3 5
→ 1
```

**gt** - Greater than
```
math gt 5 3
→ 1
```

**le** - Less than or equal
```
math le 3 5
→ 1
```

**ge** - Greater than or equal
```
math ge 5 3
→ 1
```

## Utility Operations

**max** - Maximum of two numbers
```
math max 5 3
→ 5
```

**min** - Minimum of two numbers
```
math min 5 3
→ 3
```

**abs** - Absolute value (unary)
```
math abs -5
→ 5
```

**neg** - Negate (unary)
```
math neg 5
→ -5
```

**bits** - Count 1 bits in binary
```
math bits 7
→ 3
```

**revb** - Reverse bits (32-bit)
```
math revb 1
→ -2147483648
```

## Error Handling

**Division/Modulo by zero:**
```
math div 5 0
→ Error: Division by zero!
```

**Overflow:**
```
math mul 1000000 1000000
→ Error: Multiplication overflow!
```

**Invalid input:**
```
math add abc 5
→ Error: Invalid first number
```

**Out of range:**
```
math pow 2 50
→ Error: Exponent too large (0-31)
```

## Limitations

**Range:**
- All values: -2,147,483,648 to 2,147,483,647 (32-bit signed)
- Overflow protection on mul, pow, lcm, fact, fib
- Operations wrap on overflow (except where checked)

**Factorial:**
- Maximum n = 12 (13! exceeds INT_MAX)

**Fibonacci:**
- Maximum n = 46 (47th exceeds INT_MAX)

**Power:**
- Exponent limited to 0-31 to prevent overflow

**Shifts:**
- Amount limited to 0-31 (bit width)

**Square root:**
- Negative input returns error
- Result is floor (17 → 4, not 4.123)

**Log2:**
- Positive input only
- Result is floor (17 → 4)

## Notes

**Fixed-point only:**
No floating point support. All operations use integer math.
