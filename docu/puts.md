<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Gamlang documentation - puts()</title>
</head>

<body>
<p><i>puts(str)</i></p>

<p>
Parameters:<br>
1: String
</p>

<p>
Description:<br>
Prints out parameter 1 as a string.
</p>

<p>
Code:<br>
<pre>
mov ah, 0x0F
mov bx, <dataaddr>
mov cx, <strlen>
lewp:
mov al, byte [bx]
stosw
inc bx
dec cx
jnz short lewp
</pre>
</p>

<p>
<a href="index.md">back</a><br>
</p>
</body>
</html>
