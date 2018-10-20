<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Gamlang documentation - ifequ()</title>
</head>

<body>
<p><i>ifequ(var, int, jmp)</i></p>

<p>
Parameters:<br>
1: Integer variable<br>
2: Integer
3: Jump location
</p>

<p>
Description:<br>
Checks if parameter 1 is the same as parameter 2, and if so, it jumps to parameter 3.
</p>

<p>
Code (see description):<br>
<pre>
cmp word [ptr], [var]
jz close [jmp]
</pre>
</p>

<p>
<a href="index.md">back</a><br>
</p>
</body>
</html>
