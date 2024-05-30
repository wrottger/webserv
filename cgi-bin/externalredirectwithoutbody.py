#!/usr/bin/env python

import cgi
import cgitb

cgitb.enable()  # for troubleshooting

print("Location: https://www.hellabrunn.de/tiere/welt-der-affen\r")
print("\r") # to end the CGI response headers
