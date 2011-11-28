import sys
import os

inf = sys.argv[1]
of = sys.argv[2]

in_file = open(inf, 'r')
out_file = open(of, 'w')

path = os.getcwd()

for line in in_file:
	line = line.replace("/tmp", path)
	out_file.write(line)
