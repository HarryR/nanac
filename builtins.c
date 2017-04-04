#include "nanac.h"


int reg_swp( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	nanac_reg_t tmp = nanac_reg_get(cpu, arga);
	nanac_reg_set(cpu, arga, nanac_reg_get(cpu, argb));
	nanac_reg_set(cpu, argb, tmp);
	return 0;
} 

int reg_mov( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	nanac_reg_set(cpu, arga, nanac_reg_get(cpu, argb));
	return 0;
} 

int reg_clr( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	nanac_reg_t zero = {.ptr=0};
	nanac_reg_set(cpu, arga, zero);
	nanac_reg_set(cpu, argb, zero);
	return 0;
}


int jmp_set( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->jip = nanac_uint16(arga, argb);
	return 0;
}

int jmp_to( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	if( ! jmp_set(cpu, arga, argb) ) {
		cpu->do_jump = 1;
	}
	return 0;
}

int jmp_eq( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->do_jump = nanac_reg_get(cpu, arga).ptr == nanac_reg_get(cpu, argb).ptr;
	return 0;
}

int jmp_neq( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->do_jump = nanac_reg_get(cpu, arga).ptr != nanac_reg_get(cpu, argb).ptr;
	return 0;
}

int jmp_or( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->do_jump = nanac_reg_get(cpu, arga).ptr || nanac_reg_get(cpu, argb).ptr;
	return 0;
}

int jmp_and( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->do_jump = nanac_reg_get(cpu, arga).ptr && nanac_reg_get(cpu, argb).ptr;
	return 0;
}

int jmp_die( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	return -0xFF;
}


int op_jmp( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->tmpop.op.mod = 0;  // jmp
	cpu->tmpop.op.cmd = 0;  // to
	cpu->tmpop.op.arga = arga;
	cpu->tmpop.op.argb = argb;
	return 0;
}

int op_sav( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	nanac_reg_set(cpu, arga, cpu->tmpop);
	return 0;
}

int jmp_reg( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	cpu->tmpop = nanac_reg_get(cpu, arga);
	cpu->do_tmp = 1;
	return 0;
}

int reg_win( nanac_t *cpu, uint8_t arga, uint8_t argb ) {
	if( (cpu->regs_win + arga) > 0xFF )
		return -123;
	if( (cpu->regs_win - argb) < 0 )
		return -124;
	cpu->regs_win += arga;
	cpu->regs_win -= argb;
	return 0;
}

void nanac_mods_builtins ( nanac_mods_t *mods ) {
	static const nanac_cmd_t _cmds_jmp[] = (nanac_cmd_t[]){
		{"to", &jmp_to},
		{"die", &jmp_die},
		{"set", &jmp_set},
		{"eq", &jmp_eq},
		{"neq", &jmp_neq},
		{"or", &jmp_or},
		{"and", &jmp_and},
		{"reg", &jmp_reg},
	};
	nanac_mods_add(mods, "jmp", 8, _cmds_jmp);

	static const nanac_cmd_t _cmds_op[] = (nanac_cmd_t[]){
		{"jmp", &op_jmp},
		{"sav", &op_sav},
	};
	nanac_mods_add(mods, "op", 2, _cmds_op);

	static const nanac_cmd_t _cmds_reg[] = (nanac_cmd_t[]){
		{"mov", &reg_mov},
		{"clr", &reg_clr},
		{"swp", &reg_swp},
		{"win", &reg_win},
	};
	nanac_mods_add(mods, "reg", 4, _cmds_reg);


	/* XXX: auto-determine number of commands pass in argument */
}