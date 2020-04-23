#!/usr/bin/env python
from __future__ import print_function
import sys
import re
import struct
import os
import random


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
	offset = 0
	print("""
#include <stdio.h>
#include "nanac_builtins.c"
int native_nanac (nanac_t *cpu) {
	int escape = 0;
	while( escape >= 0 ) {
		escape = 0;
#ifdef TRACE
		printf("@%d\\n", cpu->eip);
#endif
		switch( cpu->eip ) {
	""")
	for filename in sys.argv[1:]:
		with open(filename, "rb") as handle:
			cases = []
			print("/* " + filename + " */")
			while True:
				data = handle.read(4)
				if not data:
					break
				mod, cmd, arga, argb = ord(data[0]), ord(data[1]), ord(data[2]), ord(data[3])
				modstr, cmdstr = '?'+str(mod), '?'+str(cmd)
				if (mod,cmd) in instructions:
					modstr, cmdstr = instructions[(mod,cmd)]
				asmstr = "%s %s %X %X" % (modstr, cmdstr, arga, argb)
				cases.append("\n".join([
					"\t\t\tcase %d: /* %02X%02X%02X%02X %s */" % (
						offset, mod, cmd, arga, argb, asmstr),
					"\t\t\t\tescape = %s_%s(cpu, %d, %d);" % (modstr, cmdstr, arga, argb),
					"\t\t\t\tbreak;"
				]))
				offset += 1
			random.shuffle(cases)
			print("\n".join(cases))
	print("""			default: escape = -666; break;
		}
		escape = nanac_step_epilogue(cpu, escape);
	}
	return escape;
}
	""")


if __name__ == "__main__":
	main()
