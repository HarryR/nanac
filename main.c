#include "nanac.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>


static void load_file (nanac_t *cpu, const char *filename) {
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

static void print_mods (nanac_mods_t *mods) {
    for( uint8_t mod_idx = 0; mod_idx < mods->cnt; mod_idx++ ) {
        nanac_mod_t *mod = &mods->idx[mod_idx];
        for( uint8_t cmd_idx = 0; cmd_idx < mod->cmds_len; cmd_idx++ )  {
            printf("%02X %02X %s %s\n", mod_idx, cmd_idx, mod->name,
                mod->cmds[cmd_idx].name);
        }
    }
}

static int print_help (char *name) {
    fprintf(stderr, "Usage: %s [-tX] [file.bin ...]\n", name);
    fprintf(stderr, "  -t   Trace opcodes at runtime\n");
    fprintf(stderr, "  -X   Display opcode hex reference\n");
    return 10;
}

int main( int argc, char **argv ) {
    int flag_trace = 0;
    int flag_display = 0;
    int flag_help = 0;

    int opt;
    while( (opt = getopt(argc, argv, "tX")) != -1 ) {
        switch(opt) {
        case 't': flag_trace++; break;
        case 'X': flag_display++; break;
        default:
            fprintf(stderr, "error: invalid option %c\n", opt);
            flag_help = 1;
            break;
        }
    }

    if( flag_help ) {
        return print_help(argv[0]);
    }

    nanac_mods_t mods;
    nanac_mods_init(&mods);
    nanac_mods_builtins(&mods);

    if( flag_display ) {
        print_mods(&mods);
    }
    else if( optind >= argc ) {
        return print_help(argv[0]);
    }

    for( int i = optind; i < argc; i++ ) {
        nanac_t ctx;
        nanac_init(&ctx, &mods);
        load_file(&ctx, argv[i]);
        nanac_run(&ctx);
        if( ctx.ops ) {
            free(ctx.ops);
        }
    }

    return 0;
}