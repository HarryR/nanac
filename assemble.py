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


def parse_instructions():
	ops = os.popen("./nanac.exe -X").read()
	out = dict()
	for line in ops.split("\n"):
		line = line.strip()
		if not line:
			continue
		line = line.split()
		out[(line[2], line[3])] = (int(line[0],16), int(line[1],16))
	return out


class Opcode(object):
	def __init__(self, mod, cmd, arga, argb, source=None):
		self.eip = None
		self.source = source
		self.mod = mod
		self.cmd = cmd
		self.arga = arga
		self.argb = argb

	def __str__(self):
		return "@%-04X %s %s %s %s  # %s:%s" % (
			self.eip, self.mod, self.cmd,
			self.arga if self.arga else "", self.argb if self.argb else "",
			self.source[0], self.source[1]
			)

	def resolve(self, asm):
		mod, cmd = asm.ins[(self.mod, self.cmd)]

		arga = self.arga
		argb = 0
		if arga is None:
			arga = 0
		else:
			if isinstance(arga, str):
				if arga[0] == ':':
					arga = asm.labels[arga].eip
				elif arga[0] == '$':
					arga = asm.vars[arga]
			arga = int(arga)

			argb = self.argb
			if argb is None:
				argb = 0
			else:
				if isinstance(argb, str):
					if argb[0] == '$':
						argb = asm.vars[argb]
				argb = int(argb)

		return mod, cmd, arga, argb


class Assembler(object):
	def __init__(self, instructions):
		self.labels = dict()
		self.vars = dict()
		self.ir = []
		self.ops = None
		self.ins = instructions
		self.newlabel = None

	def assemble(self, line, filename=None, fileno=None):
		match = RX_LINE.match(line)
		if not match:
			raise RuntimeError("Parse error in %s:%d - %s" % (
				filename, fileno, line))
		match = match.groupdict()
		if match['var']:
			self.vars[match['var']] = int(match['val'])
		if match['label']:
			self.newlabel = match['label']
		if match['mod']:
			mod, cmd = match['mod'], match['cmd']			
			arga = match['arga']
			argb = match['argb']
			opcode = Opcode(mod, cmd, arga, argb, source=(filename, fileno))
			if self.newlabel:
				self.labels[self.newlabel] = opcode
				self.newlabel = None
			self.ir.append(opcode)

	def compile(self):
		for eip, op in enumerate(self.ir):
			op.eip = eip
		self.ops = []
		for op in self.ir:
			bytecode = op.resolve(self)
			print(("%02X%02X%02X%02X " % bytecode) + str(op))
			self.ops.append(bytecode)

	def assemble_file(self, handle):
		for idx, line in enumerate(handle):
			self.assemble(line, filename=handle.name, fileno=idx)

	def write_bytecode(self, handle):
		if self.ops is None:
			raise RuntimeError("No opcodes to be written")
		eip = 0
		for mod, cmd, arga, argb in self.ops:
			handle.write(struct.pack("<BBBB", mod, cmd, arga, argb))
			eip += 1


def main():
	instructions = parse_instructions()
	for filename in sys.argv[1:]:
		out_filename = os.path.splitext(filename)[0] + ".bin"
		asm = Assembler(instructions)
		print("\n# " + filename + " > " + out_filename)
		with open(filename) as handle:
			asm.assemble_file(handle)
		asm.compile()
		with open(out_filename, "wb") as handle:
			asm.write_bytecode(handle)



if __name__ == "__main__":
	main()