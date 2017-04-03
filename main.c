#include "nanac.h"

#include <stdio.h>

int reg_swp( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	void *tmp = cpu->regs[arga];
	cpu->regs[arga] = cpu->regs[argb];
	cpu->regs[argb] = tmp;
	return 0;
} 

int reg_mov( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->regs[arga] = cpu->regs[argb];
	return 0;
} 

int reg_clr( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->regs[arga] = 0;
	cpu->regs[argb] = 0;
	return 0;
}


int jmp_set( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->jip = arga | (argb<<8);
	return 0;
}

int jmp_to( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->eip = arga | (argb<<8);
	return 0;
}

int jmp_eq( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	if( cpu->regs[arga] == cpu->regs[arga] ) {
		cpu->eip = cpu->jip;
	}
	return 0;
}

int jmp_neq( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	if( cpu->regs[arga] != cpu->regs[arga] ) {
		cpu->eip = cpu->jip;
	}
	return 0;
}

int jmp_or( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	if( cpu->regs[arga] || cpu->regs[arga] ) {
		cpu->eip = cpu->jip;
	}
	return 0;
}

int jmp_and( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	if( cpu->regs[arga] && cpu->regs[arga] ) {
		cpu->eip = cpu->jip;
	}
	return 0;
}

int jmp_die( nanac_cpu_t *cpu, uint8_t arga, uint8_t argb ) {
	return 1;
}

static void print_mods (nanac_cpu_t *cpu) {
	for( uint8_t mod_idx = 0; mod_idx < cpu->mods_cnt; mod_idx++ ) {
		nanac_mod_t *mod = &cpu->mods[mod_idx];
		for( uint8_t cmd_idx = 0; cmd_idx < mod->cmds_len; cmd_idx++ )  {
			printf("%02X %02X %s %s\n", mod_idx, cmd_idx, mod->name,
				mod->cmds[cmd_idx].name);
		}
	}
}

int main( int argc, char **argv ) {
	nanac_cpu_t cpu;
	nanac_init(&cpu);

	nanac_addmod(&cpu, "reg", 3, (nanac_cmd_t[]){
		{"mov", &reg_mov},
		{"clr", &reg_clr},
		{"swp", &reg_swp},
	});

	nanac_addmod(&cpu, "jmp", 7, (nanac_cmd_t[]){
		{"die", &jmp_die},
		{"to", &jmp_to},
		{"set", &jmp_set},
		{"eq", &jmp_eq},
		{"neq", &jmp_neq},
		{"or", &jmp_or},
		{"and", &jmp_and},
	});

	print_mods(&cpu);

	return 0;
}