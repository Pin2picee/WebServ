#!/usr/bin/python3

import cgi
import cgitb

# Active le debug (utile pour voir les erreurs CGI dans le navigateur)
cgitb.enable()

# Récupère les données POST (si présentes)
form = cgi.FieldStorage()

# Header obligatoire
print("Content-Type: text/html")
print()  # ligne vide pour séparer headers du body

# Corps HTML
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>Hello from CGI!</h1>")

# Affiche les données POST si elles existent
if form:
    print("<h2>POST Data:</h2>")
    print("<ul>")
    for key in form.keys():
        value = form.getvalue(key)
        print(f"<li>{key} = {value}</li>")
    print("</ul>")
else:
    print("<p>No POST data received.</p>")

print("</body>")
print("</html>")
