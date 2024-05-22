import cgi
import os

form = cgi.FieldStorage()
print("HTTP/1.0 200 OK")
print("Content-type: text/html")
print()
print('<html>')
print('<body>')
file_uploaded = False
save_path = "/home/dnebatz/Documents/realwebserv/webserv/www/"  # specify your save path here

# Iterate over all fields in the form
for key in form.keys():
    fileitem = form[key]

    # Check if the field is a list
    if isinstance(fileitem, list):
        # Process each item in the list
        for item in fileitem:
            if item.filename:
                # Debug print: print the filename of the uploaded file
                print(f'<p>Uploaded file: {item.filename}</p>')
                # Strip leading path from file name to avoid directory traversal attacks
                fn = os.path.basename(item.filename)
                open(os.path.join(save_path, fn), 'wb').write(item.file.read())
                print(f'<p>File "{fn}" was uploaded successfully</p>')
    else:
        if fileitem.filename:
            # Debug print: print the filename of the uploaded file
            print(f'<p>Uploaded file: {fileitem.filename}</p>')
            # Strip leading path from file name to avoid directory traversal attacks
            fn = os.path.basename(fileitem.filename)
            open(os.path.join(save_path, fn), 'wb').write(fileitem.file.read())
            print(f'<p>File "{fn}" was uploaded successfully</p>')

print('</body>')
print('</html>')
