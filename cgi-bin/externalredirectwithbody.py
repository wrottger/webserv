#!/usr/bin/env python

import cgi
import cgitb

cgitb.enable()  # for troubleshooting

print("status: 302 Found")
print("Content-Type: text/html\r")
print("Location: https://www.new-url.com\r")
print("\r") # to end the CGI response headers

print("<html>")
print("<head>")
print("<meta http-equiv=\"refresh\" content=\"0;url=https://www.new-url.com\" />")
print("<title>You are going to be redirected</title>")
print("</head>")
print("<body>")
print("Redirecting...<a href=\"https://www.new-url.com\">Click here if you are not redirected</a>")
print("</body>")
print("</html>")