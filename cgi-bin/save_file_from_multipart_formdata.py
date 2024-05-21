#!/usr/bin/env python3

import cgi
import os
import cgitb
cgitb.enable()

form = cgi.FieldStorage()

# Directory to save files
save_path = '/home/dnebatz/Documents/realwebserv/webserv/www/'

# Go through all fields in the form
for key in form.keys():
	fileitem = form[key]

	# Check if the field is a list of files
	if isinstance(fileitem, list):
		for item in fileitem:
			if item.filename:
				# Strip leading path from file name to avoid directory traversal attacks
				fn = os.path.basename(item.filename)
				with open(save_path + fn, 'wb') as f:
					chunk = item.file.read(8192)
					while chunk:
						f.write(chunk)
						chunk = item.file.read(8192)

				print(f'The file "{fn}" was uploaded successfully<br>')
	else:
		# Check if the field is a file
		if fileitem.filename:
			# Strip leading path from file name to avoid directory traversal attacks
			fn = os.path.basename(fileitem.filename)
			with open(save_path + fn, 'wb') as f:
				chunk = fileitem.file.read(8192)
				while chunk:
					f.write(chunk)
					chunk = fileitem.file.read(8192)

			print(f'The file "{fn}" was uploaded successfully<br>')

print("""\
Content-Type: text/html\n
<html><body>
<p>All files uploaded successfully</p>
</body></html>
""")