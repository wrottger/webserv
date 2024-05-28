import cgi
import os

form = cgi.FieldStorage()
print("Status: 200 OK")
print("Content-type: text/html")
print()
print('<html>')
print('<body>')
file_uploaded = False
save_path = os.path.join("/home", os.getenv("USER"), "sgoinfre")

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
                chunk_size = 4096  # Or any other chunk size you want
                with open(os.path.join(save_path, fn), 'wb') as f:
                    while True:
                        chunk = item.file.read(chunk_size)
                        if not chunk:
                            break
                        f.write(chunk)
                print(f'<p>File "{fn}" was uploaded successfully</p>')
    else:
        if fileitem.filename:
            # Debug print: print the filename of the uploaded file
            print(f'<p>Uploaded file: {fileitem.filename}</p>')
            # Strip leading path from file name to avoid directory traversal attacks
            fn = os.path.basename(fileitem.filename)
            chunk_size = 4096  # Or any other chunk size you want
            with open(os.path.join(save_path, fn), 'wb') as f:
                while True:
                    chunk = fileitem.file.read(chunk_size)
                    if not chunk:
                        break
                    f.write(chunk)
print(f'<p>File "{fn}" was uploaded successfully</p>')
print('</body>')
print('</html>')