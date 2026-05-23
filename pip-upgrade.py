#!/usr/bin/python
import subprocess
print("Waiting for output...\n")
result = subprocess.run(["pip", "list","--outdated"], capture_output=True, text=True).stdout.split("\n")[2:]
for i in result:
    subprocess.run(["pip","install","--upgrade",i.split(" ")[0],"--break-system-packages"])

