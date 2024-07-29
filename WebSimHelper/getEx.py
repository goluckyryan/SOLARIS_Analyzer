#!/usr/bin/env /usr/bin/python3

import isotopeLib as iso

import sys

if len(sys.argv) < 3:
  print("Usage: python getEx.py A Z Ex")
  sys.exit(1)

A = int(sys.argv[1])
Z = int(sys.argv[2])
Ex = float(sys.argv[3])
ASym = iso.GetSymbol(A, Z)

ExList = iso.GetExList(ASym, Ex)

print(ExList)