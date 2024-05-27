#!/usr/bin/env python

import cgi
import cgitb

cgitb.enable()  # for troubleshooting

print("Location: /test/index.html")
print() # to end the CGI response headers
