#!/usr/bin/python3

import isotopeLib
import cgi

form = cgi.FieldStorage()

ASym = form.getvalue('ASym')
A=form.getvalue('A')
Z=form.getvalue('Z')


print( "Content-type:text/html\r\n\r\n")

if 'ASym' in form :

  if ASym == "p" : ASym = "1H"
  if ASym == "d" : ASym = "2H"
  if ASym == "t" : ASym = "3H"
  if ASym == "a" : ASym = "4He"

  [A,Z] = isotopeLib.GetAZ(ASym)

if 'A' in form:
  ASym = isotopeLib.GetSymbol(float(A), float(Z))

#===================================
# print(A)
# print(Z)
# print(isotopeLib.GetMassFromSym(ASym))
# print(ASym)
# print(isotopeLib.GetSn(ASym))
# print(isotopeLib.GetSp(ASym))
# print(isotopeLib.GetSa(ASym))

print(A , ",",
      Z , ",",
      format(isotopeLib.GetMassFromSym(ASym), '.2f'), ",",
      ASym, ",", 
      format(isotopeLib.GetSn(ASym), '.2f'), ",",
      format(isotopeLib.GetSp(ASym), '.2f'), ",",
      format(isotopeLib.GetSa(ASym), '.2f')
      )
