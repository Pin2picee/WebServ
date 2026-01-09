#!/usr/bin/env python3
import sys
import os
import urllib.parse

# DEBUG
sys.stderr.write(f"[CGI] PID={os.getpid()} CONTENT_LENGTH={os.environ.get('CONTENT_LENGTH', 'NOT SET')}\n")
sys.stderr.flush()

# --- Lire les données ---
method = os.environ.get("REQUEST_METHOD", "GET").upper()
input_text = ""

if method == "POST":
    try:
        length = int(os.environ.get("CONTENT_LENGTH", 0))
    except (TypeError, ValueError):
        length = 0
    sys.stderr.write(f"[CGI] About to read {length} bytes\n")
    sys.stderr.flush()
    if length > 0:
        input_text = sys.stdin.read(length)
    sys.stderr.write(f"[CGI] Read: '{input_text}'\n")
    sys.stderr.flush()
elif method == "GET":
    input_text = os.environ.get("QUERY_STRING", "")

# Décoder les données URL-encoded
input_text = urllib.parse.unquote_plus(input_text.replace("&", "<br>"))

if not input_text:
    input_text = "<i>Aucune donnée reçue.</i>"

# --- Génération HTML ---
HTML = f"""<!doctype html>
<html lang="fr">
<head>
<meta charset="UTF-8" />
<title>CGI Echo Input</title>
<style>
body {{ font-family: sans-serif; padding: 2em; background-color: #111827; color: #f8fafc; }}
.container {{ background-color: rgba(17,24,39,0.6); padding: 20px; border-radius: 12px; }}
h1 {{ color: #0ea5e9; }}
</style>
</head>
<body>
<div class="container">
<h1>Données reçues par le CGI</h1>
<p>{input_text}</p>
</div>
</body>
</html>
"""

# --- En-têtes HTTP ---
sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n\r\n")
sys.stdout.write(HTML)