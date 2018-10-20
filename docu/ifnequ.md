# Gamlang documentation - ifnequ()

ifnequ(var, int, jmp)

| Parameter | Description |
| --------|--------|
| 1 | Integer variable |
| 2 | Integer |
| 3 | Jump location |

## Description

Checks if parameter 1 is the same as parameter 2, and if it is not, it jumps to parameter 3.

## Code (see description)

```
cmp word [ptr], [var]
jnz close [jmp]
```
