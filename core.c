#include "nanac.h"

#include <string.h>
#include <stdio.h>



void nanac_init( nanac_t *cpu ) {
	memset(cpu, 0, sizeof(*cpu));
}

int nanac_addmod( nanac_t *cpu, const char *name, uint8_t cmds_len, const nanac_cmd_t cmds[] ) {
	if( cpu->mods_cnt >= 0xFF )
		return 0;
	nanac_mod_t *mod = &cpu->mods[cpu->mods_cnt++];
	mod->name = name;
	mod->cmds_len = cmds_len;
	mod->cmds = cmds;
	return 1;
}

const nanac_op_t *nanac_op( const nanac_t *cpu, const uint16_t eip ) {
	if( eip < cpu->ops_sz ) {
		return &cpu->ops[eip];
	}
	return NULL;
}

int nanac_step( nanac_t *cpu, const nanac_op_t *op ) {
	const nanac_mod_t *mod = &cpu->mods[op->mod];
	if( ! mod ) {
		return -11;
	}

	if( op->cmd >= mod->cmds_len )
		return -12;

	const nanac_cmd_t *cmd = &mod->cmds[op->cmd];

#ifdef TRACE
	printf("@%-4X %s %s %d %d\n", cpu->eip, mod->name, cmd->name, op->arga, op->argb);
#endif

	int ret = cmd->run(cpu, op->arga, op->argb);
	if( ! ret ) {
		if( cpu->do_jump ) {
			cpu->eip = cpu->jip;
			cpu->do_jump = 0;
		}
		else cpu->eip += 1;
	}
	return ret;
}

int nanac_run( nanac_t *cpu ) {
	int escape = 0;
	while( escape >= 0 ) {
		const nanac_op_t *op = nanac_op( cpu, cpu->eip );
		if( ! op ) {
			escape = -10;
			break;
		}
		escape = nanac_step( cpu, op );
	}
	return escape;
}

const uint8_t *nanac_bytes( const nanac_t *cpu, uint32_t offset, uint32_t length ) {
	if( (offset + length) > (cpu->ops_sz * 4) )
		return NULL;
	return &((uint8_t*)cpu->ops)[offset];
}
