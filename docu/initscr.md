<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Gamlang documentation - initscr()</title>
</head>

<body>
<p><i>initscr()</i></p>

<p>
Parameters:<br>
No parameters
</p>

<p>
Description:<br>
Sets up the screen by setting the mode to text mode, sets es to 0xB800, and clears di.
</p>

<p>
Code:<br>
<pre>
mov di, 0xB800
mov es, di
xor di, di
mov ax, 0x0003
int 0x10
</pre>
</p>

<p>
<a href="index.md">back</a><br>
</p>
</body>
</html>
