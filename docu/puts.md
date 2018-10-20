# Gamlang documentation - puts()

puts(str)

| Parameter | Description |
| --------|--------|
| 1 | String |

## Description

Prints out parameter 1 as a string.

## Code (see description)

```
mov ah, 0x0F
mov bx, <dataaddr>
mov cx, <strlen>
lewp:
mov al, byte [bx]
stosw
inc bx
dec cx
jnz short lewp
```
