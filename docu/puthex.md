# Gamlang documentation - puthex()

puthex(int)

| Parameter | Description |
| --------|--------|
| 1 | Integer |

## Description

Prints out parameter 1 in hex.

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
