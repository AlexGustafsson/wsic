#!/usr/bin/env bash

cat <<EOF
HTTP/1.1 200 OK
Content-type: text/html

EOF

cat <<EOF
<!doctype html>
<html>
<head>
	<meta charset='utf8'>
	<meta name='viewport' content='width=device-width'>

	<title>WSIC</title>

	<link rel='stylesheet' href='../style.css'>
</head>

<body>
	<header>
	  <a href="/"><h1>W[!]</h1></a>
	</header>
	<main>
    <h1>WSIC CGI test page</h1>
    <p>WSIC has support for CGI which enables interactive websites using Bash, PHP and other technologies.</p>
		<p>This page uses Bash in order to provide a more interactive experience.</p>
		<h2>Demos</h2>
		<ul>
			<li><a href="/cgi/form">Form (POST)</a></li>
			<li><a href="/cgi/environment">Supported environment variables</a></li>
		</ul>
	</main>
	<footer>
	  <p>This page was served by WSIC at <code>$(date)</code></p>
		<p>WSIC was crafted with love by <a href="https://github.com/AlexGustafsson">Alex Gustafsson</a> and <a href="https://github.com/MarcusLenander">Marcus Lenander</a></p>
		<p><a href="https://github.com/AlexGustafsson/wsic"><span class="github"></span></a></p>
	</footer>
</body>
</html>
EOF

exit 0
