#include "nanac.h"
int native_nanac (nanac_t *cpu);
int main( int argc, char **argv ) {
	nanac_mods_t mods;
    nanac_mods_init(&mods);
    nanac_mods_builtins(&mods);

    nanac_t ctx;
    nanac_init(&ctx, &mods);
    return native_nanac(&ctx);
}