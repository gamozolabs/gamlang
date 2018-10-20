# Gamlang documentation - storestr()

storestr(var, str)

| Parameter | Description |
| --------|--------|
| 1 | String variable |
| 2 | String |

## Description

This function creates a string variable, initialized to str when the variable doesn't exist. If the variable does exist, it will modify the 
current string to str (str will fit in the variable), and the code is only placed in the binary if the variable was previously used, otherwise
the variable is hardcoded to the most recent storestr() call, prior to any usage of it.

## Code (see description)

```
mov bx, <replace>
mov cx, <count to replace>
mov bp, <replace with>
.lewp:
mov al, [bp]
mov [bx], al
inc bx
inc bp
dec cx
jmp short .lewp
```
