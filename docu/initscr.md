# Gamlang documentation - initscr()

initscr()

## Description

Sets up the screen by setting the mode to text mode, sets es to 0xB800, and clears di.

## Code

```
mov di, 0xB800
mov es, di
xor di, di
mov ax, 0x0003
int 0x10
```
