#!/usr/bin/env python3
import os
import sys
import time

print("Content-type: text/html")
print()
print("<html><head>")
print("<title>Hello CGI Program</title>")
print("</head><body>")
print("<h2>Hello World! This is a Python CGI script.</h2>")
print("<h3>Environment Variables:</h3>")
print("<pre>")
for key, value in os.environ.items():
    print(f"{key}: {value}")
print("</pre>")
print("</body></html>")