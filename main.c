// find a seed with a certain structure at the origin chunk
#include "finders.h"
#include <stdio.h>

int main()
{
    initBiomes();

    // Note that some structure configs changed with the Minecraft versions.
    const StructureConfig sconf = OUTPOST_CONFIG;
    int mc = MC_1_16;

    LayerStack g;
    setupGenerator(&g, mc);

    int64_t lower48;
    for (lower48 = 0; ; lower48++)
    {
        // The structure position depends only on the region coordinates and
        // the lower 48-bits of the world seed.
        int valid;
        Pos p = getStructurePos(sconf, lower48, 0, 0, &valid);

        // Look for a seed with the structure at the origin chunk.
        if (!valid || p.x >= 16 || p.z >= 16)
            continue;

        // Look for a full 64-bit seed with viable biomes.
        int64_t upper16;
        for (upper16 = 0; upper16 < 0x10000; upper16++)
        {
            int64_t seed = lower48 | (upper16 << 48);
            if (isViableStructurePos(sconf.structType, mc, &g, seed, p.x, p.z))
            {
                printf("Seed %" PRId64 " has a Pillager Outpost at the origin.\n",
                    seed, p.x, p.z);
                return 0;
            }
        }
    }
}