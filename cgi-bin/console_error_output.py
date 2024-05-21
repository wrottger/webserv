import time

code = """
import sys

for _ in range(4):
    print("ERROR CHILD", file=sys.stderr)
"""

with open('output.py', 'a') as f:
	for _ in range(4):
		f.write("print('ERROR PARENT')\n")