#!/usr/bin/python
import sys
import os
import random

if len(sys.argv)<3:
    print("Usage: ./random-text.py <file-name> <file-length>")
    sys.exit(1)

os.path.exists(sys.argv[1]) and print("Error: <file-name> must be a non-exist path")==None and sys.exit(1)

times: int=int(sys.argv[2] if sys.argv[2].isdigit() else (sys.exit(1) if print("Error: <file-length> must be an available dec")==None else None))

LETTERS: str="".join(i for i in range("a","z"+1))+"".join(i for i in range("A","Z"+1))+"0123456789, ;.?"
def random_text():
    return "\n".join("".join(random.choice(LETTERS) for n in range(i)) for i in range(random.randint(20,50)))

with open(sys.argv[1],"a+") as file:
    for i in range(times):
        print(f"line {i}:",random_text(),file=file)
    print(file=file,flush=True)
