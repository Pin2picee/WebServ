#!/usr/bin/python3

import os
import sys

# Header obligatoire
print("Content-Type: text/html")
print()  # ligne vide séparant headers et body

# Corps HTML
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>Hello from CGI!</h1>")

# Affiche les variables d'environnement CGI
print("<h2>Environment Variables:</h2>")
print("<ul>")
print(f"<li>REQUEST_METHOD: {os.environ.get('REQUEST_METHOD', 'N/A')}</li>")
print(f"<li>QUERY_STRING: {os.environ.get('QUERY_STRING', 'N/A')}</li>")
print(f"<li>CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', 'N/A')}</li>")
print(f"<li>CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', 'N/A')}</li>")
print("</ul>")

# Pour lire le POST body (si présent)
content_length = os.environ.get('CONTENT_LENGTH')
if content_length:
    post_data = sys.stdin.read(int(content_length))
    print(f"<h2>POST Data:</h2>")
    print(f"<pre>{post_data}</pre>")

print("</body>")
print("</html>")