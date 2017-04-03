#include "builtins.h"


static int reg_swp( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	void *tmp = cpu->regs[arga];
	cpu->regs[arga] = cpu->regs[argb];
	cpu->regs[argb] = tmp;
	return 0;
} 

static int reg_mov( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->regs[arga] = cpu->regs[argb];
	return 0;
} 

static int reg_clr( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->regs[arga] = 0;
	cpu->regs[argb] = 0;
	return 0;
}


static int jmp_set( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->jip = nanac_uint16_t(arga, argb);
	return 0;
}

static int jmp_to( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	if( ! jmp_set(cpu, arga, argb) ) {
		cpu->do_jump = 1;
	}
	return 0;
}

static int jmp_eq( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->do_jump = cpu->regs[arga] == cpu->regs[arga];
	return 0;
}

static int jmp_neq( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->do_jump = cpu->regs[arga] != cpu->regs[arga];
	return 0;
}

static int jmp_or( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->do_jump = cpu->regs[arga] || cpu->regs[arga];
	return 0;
}

static int jmp_and( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->do_jump = cpu->regs[arga] && cpu->regs[arga];
	return 0;
}

static int jmp_die( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	return -0xFF;
}

static const nanac_cmd_t _cmds_reg[] = (nanac_cmd_t[]){
	{"mov", &reg_mov},
	{"clr", &reg_clr},
	{"swp", &reg_swp},
};

static const nanac_cmd_t _cmds_jmp[] = (nanac_cmd_t[]){
	{"die", &jmp_die},
	{"to", &jmp_to},
	{"set", &jmp_set},
	{"eq", &jmp_eq},
	{"neq", &jmp_neq},
	{"or", &jmp_or},
	{"and", &jmp_and},
};

void nanac_builtins ( nanac_cpu_t *cpu ) {
	nanac_addmod(cpu, "reg", 3, _cmds_reg);
	nanac_addmod(cpu, "jmp", 7, _cmds_jmp);
}