#!/usr/bin/env python3
import os
import sys
import time
# Get the CONTENT_LENGTH from the environment variables
content_length = int(os.getenv('CONTENT_LENGTH', 0))
buffer = sys.stdin.read(content_length)

print("status: 200 OK")
print("content-type: text/html\r")
print("\r")
print("<html><head>")
print("<title>Hello CGI Program</title>")
print("</head><body>")
print("<h2>Hello World! This is a Python CGI script.</h2>")
print("<h3>Environment Variables:</h3>")
print("<pre>")
for key, value in os.environ.items():
    print(f"{key}: {value}")
print("</pre>")
print(buffer)
print("</body></html>")
print()
