#!/usr/bin/env bash

request="$(cat)"

cat <<EOF
HTTP/1.1 200 OK
Content-type: text/html

EOF

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

      footer {
        border-top: 1px solid #DCDCDC;
      }
    </style>
  </head>
  <body>
    <center>
      <h1>Hello world from Bash CGI!</h1>
    </center>
    <b>Request:</b><br />
    <code>$request</code><br />
    <b>Environment variables:</b><br />
    <code>$(env)</code>
    <footer>
      <center>
        <p>Built by Bash at <code>$(date)</code></p>
      </center>
    </footer>
  </body>
</html>
EOF

exit 0
