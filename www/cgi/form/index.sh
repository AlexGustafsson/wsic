#!/usr/bin/env bash

if [[ "$REQUEST_METHOD" = "POST" ]]; then
  body="$(cat)"
fi

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
    <h1>WSIC CGI Form</h1>
    <form method="POST" action="/cgi/form">
      <input type="text" name="name" placeholder="Name" />
      <input type="submit" value="Submit" />
    </form>
    <h2>Result</h2>
    <code>$body</code>
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
