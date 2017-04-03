#!/usr/bin/env python
from __future__ import print_function
import sys
import re
import struct
import os


RX_LINE = re.compile(r"""
^
(
	(
	 ((?P<label>:[a-zA-Z0-9]+))?
	 (\s*\b
		(?P<mod>[a-z]+)\s+
		(?P<cmd>[a-z]+)
		(\s+
			(?P<arga>[:\$]?[a-zA-Z0-9]+)
			(\s+
				(?P<argb>\$?[a-zA-Z0-9]+)
			)?
		)?
	 )?
	)
	|((?P<var>\$[a-zA-Z0-9]+)\s+(?P<val>[0-9]+))	
)?
(\s*\#.*)?
$
""", re.VERBOSE)


def parse_ops():
	ops = os.popen("./nanac.exe").read()
	out = dict()
	for line in ops.split("\n"):
		line = line.strip()
		if not line:
			continue
		line = line.split()
		out[(line[2], line[3])] = (int(line[0],16), int(line[1],16))
	return out


def main():
	ops = parse_ops()

	namedregs = dict()
	labels = dict()
	newlabel = None
	args = sys.argv[1:]
	offset = 0
	for filename in args:
		out_filename = os.path.splitext(filename)[0] + ".bin"
		with open(out_filename, "wb") as outhan:
			with open(filename) as handle:
				lineno = 1
				for line in handle:
					line = line.strip()
					if len(line):
						match = RX_LINE.match(line)
						print("%5d %s" %(lineno, line))
						if match:
							match = match.groupdict()
							if match['var']:
								namedregs[match['var']] = int(match['val'])
							if match['label']:
								newlabel = match['label']
							if match['mod']:
								if newlabel:
									labels[newlabel] = (offset & 0xFF), (offset >> 8)
									newlabel = None
								mod, cmd = ops[(match['mod'], match['cmd'])]
								arga = match['arga']
								argb = match['argb']
								if arga and arga[0] == ':':
									arga, argb = labels[arga]
								else:
									if arga:
										if arga[0] == '$':
											arga = namedregs[arga]
										else:
											arga = int(arga)
									else:
										arga = 0											
									if argb:
										if argb[0] == '$':
											argb = namedregs[argb]
										else:
											argb = int(argb)
									else:
										argb = 0
								print("@%-4X %02X %02X %02X %02X" % (offset, mod, cmd, arga, argb))
								outhan.write(struct.pack("<BBBB", mod, cmd, arga, argb))
								offset += 1
					lineno += 1


if __name__ == "__main__":
	main()