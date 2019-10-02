#!/usr/bin/env bash

cat <<EOF
HTTP/1.1 200 OK
Content-type: text/html

EOF

if [[ "$REQUEST_METHOD" == "POST" ]]; then
  body="$(cat)"
fi

cat <<EOF
<!DOCTYPE html>
<html lang="en" dir="ltr">
  <head>
    <meta charset="utf-8">
    <title>WSIC</title>
    <style>
      h1, p {
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen, Ubuntu, Cantarell, "Fira Sans", "Droid Sans", "Helvetica Neue", Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol";
      }

      header {
        border-bottom: 1px solid #DCDCDC;
      }

      main {
        padding: 10px 0;
      }

      footer {
        border-top: 1px solid #DCDCDC;
      }
    </style>
  </head>
  <body>
    <header>
      <center>
        <h1>Hello world from Bash CGI!</h1>
      </center>
      <b>Method:</b><br />
      <code>$REQUEST_METHOD</code><br />
      <b>Request:</b><br />
      <code>$body</code><br />
      <b>Environment variables:</b><br />
      <code>$(env)</code>
    </header>
    <main>
      <form method="post" action="/">
        <label for="name">Name</label>
        <input id="name" name="name" type="text" />
        <input type="submit" />
      </form>
    </main>
    <footer>
      <center>
        <p>Built by Bash at <code>$(date)</code></p>
      </center>
    </footer>
  </body>
</html>
EOF

exit 0
