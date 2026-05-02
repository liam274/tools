#!/usr/bin/python
import sys
import os
import random

if len(sys.argv)<3:
    print("Usage: ./random-text.py <file-name> <file-length>")
    sys.exit(1)

FILENAME: str=sys.argv[1]
FILELENGTH: str=sys.argv[2]

result: str=""
os.path.exists(FILENAME) and print("Error: <file-name> must be a non-exist path",end="")==None and (result:=input("(c for \"cover\", q for \"quit\")?"))!="c" and sys.exit(1)
result=="c" and open(FILENAME, "w").close() and print(f"File {FILENAME} cleared")

times: int=int(FILELENGTH if FILELENGTH.isdigit() else (sys.exit(1) if print("Error: <file-length> must be an available dec")==None else None))

LETTERS: str="".join(chr(i) for i in range(ord("a"),ord("z")+1))+"".join(chr(i) for i in range(ord("A"),ord("Z")+1))+"0123456789, ;.?"
def random_text():
    return "".join(random.choice(LETTERS) for i in range(random.randint(20,500)))

with open(FILENAME,"a+") as file:
    for i in range(times):
        print(f"line {i}:",random_text(),file=file)
    print(file=file,flush=True)
