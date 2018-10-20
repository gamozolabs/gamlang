<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Gamlang documentation - storestr()</title>
</head>

<body>
<p><i>storestr(var, str)</i></p>

<p>
Parameters:<br>
1: String Variable<br>
2: String
</p>

<p>
Description:<br>
This function creates a string variable, initialized to str when the variable doesn't exist. If the variable does exist, it will modify the 
current string to str (str will fit in the variable), and the code is only placed in the binary if the variable was previously used, otherwise
the variable is hardcoded to the most recent storestr() call, prior to any usage of it.
</p>

<p>
Code (see description):<br>
<pre>
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
</pre>
</p>

<p>
<a href="index.md">back</a><br>
</p>
</body>
</html>
