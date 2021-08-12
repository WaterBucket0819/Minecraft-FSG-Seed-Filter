// find a seed with a certain structure at the origin chunk
#include "finders.h"
#include <stdio.h>
#include <stdlib.h>

uint64_t rand64()
{
  uint64_t rv = 0;
  int c,i;
  FILE *fp;
  fp = fopen("/dev/urandom", "r");

  for (i=0; i < sizeof(rv); i++) {
     do {
       c = fgetc(fp);
     } while (c < 0);
     rv = (rv << 8) | (c & 0xff);
  }
  fclose(fp);
  return rv;
}

char netherchecker(uint64_t seed){
  unsigned long modulus = 1UL << 48;
  unsigned long AA = 341873128712;
  unsigned long BB = 132897987541;
  
  //the structure in pos/pos quadrant is within 8*16 (128) of 0,0
  if ( ( ( ( (0x5deece66dUL*((seed + 30084232UL) ^ 0x5deece66dUL) + 11) % modulus ) >> 17 ) % 23) > 8){
    return 0;
  }
  if (((( 0x5deece66dUL* (0x5deece66dUL*((seed + 30084232UL) ^ 0x5deece66dUL) + 11) + 11) % modulus ) >> 17 ) % 23 > 8){
    return 0;
  }

  //now check the neg/pos quadrant (0 <= pos coord <= 128 and -128 <= neg coord <= -64 )
  if ( ( ( ( (0x5deece66dUL*((seed + 30084232UL - AA) ^ 0x5deece66dUL) + 11) % modulus ) >> 17 ) % 23) < 19){
    return 0;
  }
  if (((( 0x5deece66dUL* (0x5deece66dUL*((seed + 30084232UL - AA) ^ 0x5deece66dUL) + 11) + 11) % modulus ) >> 17 ) % 23 > 8){
    return 0;
  }

    //now check the pos/neg quadrant (0 <= pos coord <= 128 and -128 <= neg coord <= -64 )
  if ( ( ( ( (0x5deece66dUL*((seed + 30084232UL - BB) ^ 0x5deece66dUL) + 11) % modulus ) >> 17 ) % 23) > 8){
    return 0;
  }
  if (((( 0x5deece66dUL* (0x5deece66dUL*((seed + 30084232UL - BB) ^ 0x5deece66dUL) + 11) + 11) % modulus ) >> 17 ) % 23 < 19){
    return 0;
  }

  //not bothering with the neg/neg quadrant
  return 1;
}

int main()
{
    printf("finding you a random good seed for RGSG category...\n");
    initBiomes();

    // Note that some structure configs changed with the Minecraft versions.
    const StructureConfig sconf = VILLAGE_CONFIG;
    const StructureConfig sconf_portal = RUINED_PORTAL_CONFIG;
    int mc = MC_1_16;

    LayerStack g;
    setupGenerator(&g, mc);
    int seedcounter = 1;
    uint64_t lower48 = rand64() >> 16;
    for (; ; lower48=rand64() >> 16)
    {
        if (seedcounter % 100000 == 0){
          printf("tested %d seeds so far\n", seedcounter);
        }
        // The structure position depends only on the region coordinates and
        // the lower 48-bits of the world seed.
        int valid;
        Pos p = getStructurePos(sconf, lower48, 0, 0, &valid);

        // The structure position depends only on the region coordinates and
        // the lower 48-bits of the world seed.
        int valid2;
        Pos p2 = getStructurePos(sconf_portal, lower48, 0, 0, &valid);


        // Look for a seed with the structures at the origin chunk.
        if (!valid || p.x >= 96 || p.z >= 96 || p2.x >= 96 || p2.z >= 96 || !netherchecker(lower48) ){

            seedcounter++;
            continue;
        }

        // Look for a full 64-bit seed with viable biomes.
        uint64_t upper16 = rand64() >> 48;
        for (; ; upper16 = rand64() >> 48)
        {
            int64_t seed = lower48 | (upper16 << 48);
            if (isViableStructurePos(sconf.structType, mc, &g, seed, p.x, p.z) && isViableStructurePos(sconf_portal.structType, mc, &g, seed, p2.x, p2.z))
            {
                printf("Seed %ld is a good seed (close vilage, close nether portal, 3ish close nether structures).\n",
                    seed);
                printf("Examined %d seeds\n", seedcounter);
                return 0;
            }
            seedcounter++;
        }
    }
}
