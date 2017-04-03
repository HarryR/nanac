#include "nanac.h"
#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>

static void print_mods (nanac_cpu_t *cpu) {
	for( uint8_t mod_idx = 0; mod_idx < cpu->mods_cnt; mod_idx++ ) {
		nanac_mod_t *mod = &cpu->mods[mod_idx];
		for( uint8_t cmd_idx = 0; cmd_idx < mod->cmds_len; cmd_idx++ )  {
			printf("%02X %02X %s %s\n", mod_idx, cmd_idx, mod->name,
				mod->cmds[cmd_idx].name);
		}
	}
}

static void load_file (nanac_cpu_t *cpu, const char *filename) {
	FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen ( filename , "rb" );
    if( !fp ) {
    	perror(filename);
    	exit(1);
    }

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );

    if( !buffer ) {
    	fclose(fp);
    	fputs("memory alloc fails",stderr);
    	exit(1);
    }

    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) ) {
      fclose(fp);
      free(buffer);
      fputs("entire read fails",stderr);
      exit(1);
    }

    /* do your work here, buffer is a string contains the whole text */
  	cpu->ops = (nanac_op_t*)buffer;
    cpu->ops_sz = lSize / 4;
    fclose(fp);
}

int main( int argc, char **argv ) {
	nanac_cpu_t cpu;
	nanac_init(&cpu);
	nanac_builtins(&cpu);

	print_mods(&cpu);

	if( argc > 1 ) {
		load_file(&cpu, argv[1]);
	}

	nanac_run(&cpu);

	if( cpu.ops ) {
		free(cpu.ops);
	}

	return 0;
}