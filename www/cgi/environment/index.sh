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
	  <h1>W[!]</h1>
	</header>
	<main>
    <h1>WSIC CGI Environment</h1>
    <p>This is the result of calling <code>env</code>.</p>
    <div class="code-block">
    $(env)
    </div>
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
