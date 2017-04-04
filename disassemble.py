#!/usr/bin/env python
from __future__ import print_function
import sys
import re
import struct
import os


def parse_instructions():
	ops = os.popen("./nanac.exe -X").read()
	out = dict()
	for line in ops.split("\n"):
		line = line.strip()
		if not line:
			continue
		line = line.split()
		names = (line[2], line[3])
		nums = (int(line[0],16), int(line[1],16))
		out[nums] = names
	return out


def main():
	instructions = parse_instructions()
	for filename in sys.argv[1:]:
		with open(filename, "rb") as handle:
			print("\n# " + filename)
			offset = 0
			while True:
				data = handle.read(4)
				if not data:
					break
				mod, cmd, arga, argb = ord(data[0]), ord(data[1]), ord(data[2]), ord(data[3])
				modstr, cmdstr = '?'+str(mod), '?'+str(cmd)
				if (mod,cmd) in instructions:
					modstr, cmdstr = instructions[(mod,cmd)]
				asmstr = "%s %s %X %X" % (modstr, cmdstr, arga, argb)
				print("%02X%02X%02X%02X @%-4X  %s" % (mod, cmd, arga, argb, offset, asmstr))
				offset += 1


if __name__ == "__main__":
	main()
