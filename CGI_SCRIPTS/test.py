#!/usr/bin/env python3

import os
import time


os.system("curl -i http://127.0.0.1:8000/CGI_SCRIPTS/test.py &> /dev/null")
# Print CGI headers
print("Content-Type: text/html")
print()

# Print HTML response
print("<!DOCTYPE html>")
print("<html><head><title>Test CGI</title></head><body>")
print("<h1>CGI Test Successful!</h1>")
print("<p>Current time:", time.strftime("%Y-%m-%d %H:%M:%S"), "</p>")
print("<p>Script executed successfully!</p>")
print("</body></html>")