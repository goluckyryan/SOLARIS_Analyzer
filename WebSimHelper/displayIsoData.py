#!/usr/bin/env /usr/bin/python3

import isotopeLib
import cgi

form = cgi.FieldStorage()

ASym = form.getvalue('ASym')
maxEx = form.getvalue('maxEx')

print( "Content-type:text/html\r\n\r\n")
print("<html>")
print("<style> body { font-family: courier, courier new, serif; color: #F7CF3C; } </style>")
print("<body>")

isotopeLib.PrintIsoWeb(ASym)

if maxEx == "can be omitted" or float(maxEx) <= 0:
  maxEx = -1
else:
  isotopeLib.PrintIsoExWeb(ASym, float(maxEx))


print("</body>")
print("</html>")




