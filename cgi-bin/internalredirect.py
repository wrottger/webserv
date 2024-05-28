#!/usr/bin/env python

import cgi
import cgitb

cgitb.enable()  # for troubleshooting

print("Location: /test/index.html\r")
print("\r") # to end the CGI response headers
