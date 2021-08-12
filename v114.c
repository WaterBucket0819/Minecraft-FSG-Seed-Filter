#include "finders.h"
#include <stdio.h>
#include <stdlib.h>
#include <gcrypt.h>
#define MAXBUFLEN 60000
#define VERSION 99 //id for the validation
#define FILTER_TYPE 4// 0=> RSG overworld (Practice Seeds); 1=> village only; 2=> shipwreck only; 3 => jungle only; 4 => coinflip (classic); 5=> whatever I want; 6 => loot testing
#define DEBUG 0
#include <string.h>
#include <time.h>
#include "./minecraft_nether_gen_rs.h"


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

long distance_squared(long x1, long z1, long x2, long z2){
  return ((x1-x2)*(x1-x2) + (z1-z2)*(z1-z2)); //l2norm
}

int fortressCheck(int64_t seed, long cutoff){ 
  //cutoff distance squared and if I find any that close then return 1
  int64_t dummyseed;
  int doesExist, chunkx, chunkz;
  int regx, regz;
  long fx, fz;
  for (regx = -1; regx <= 0; regx++){
    for (regz = -1; regz <= 0; regz++){
      dummyseed = (regx) ^ (regz << 4) ^ (seed) % (1L << 48);
      dummyseed = dummyseed ^ 0x5deece66dUL;
      nextInt(&dummyseed, 5);
      doesExist = nextInt(&dummyseed, 3) == 0;
      chunkx = nextInt(&dummyseed, 8) + 4;
      chunkz = nextInt(&dummyseed, 8) + 4;  
      if (doesExist){
        fx = regx*256 + (chunkx << 4) + 11;
        fz = regz*256 + (chunkz << 4) + 11;
        if (distance_squared(fx, fz, 0L, 0L) <= cutoff){
          return 1;
        }
      }
    }
  }
  return 0;
}

int desertTemple(int64_t seed, long cutoff, int* txp, int* tzp){
  const StructureConfig sconf = DESERT_PYRAMID_CONFIG;
  int valid;
  int regx, regz;
  long tx,tz;
  Pos p;
  for (regx = -1; regx <= 0; regx++){
    for (regz = -1; regz <= 0; regz++){
      p = getStructurePos(sconf, seed, regx, regz, &valid);
      if (valid){
        tx = p.x + 9;
        tz = p.z + 9;
        if (distance_squared(tx,tz,0,0) <= cutoff){
          *txp = tx;
          *tzp = tz;
          return 1;
        } //should I save the region
        //printf("regx,z %d %d coords %d %d\n", regx, regz, p.x + 9, p.z + 9);
      }
    }
  }
  return 0;
}

int lava_non_desert(int64_t lower48, int x, int z, int* lx, int* lz){
  //printf("lower48: %ld\n", lower48);
  int64_t fakeseed = (lower48) ^ 0x5deece66dUL;
  long a = nextLong(&fakeseed) | 1;
  long b = nextLong(&fakeseed) | 1;
  fakeseed = ((long)x * a + (long)z * b) ^ lower48;
  fakeseed = fakeseed & 0xffffffffffff; //population seed set?
  //printf("population seed: %ld\n", fakeseed);
  int64_t lakeseed = (fakeseed + 10000) ^ 0x5deece66dUL;
  //printf("lakeseed: %ld\n", lakeseed);
  int64_t lavaseed = (fakeseed + 10001) ^ 0x5deece66dUL;//10001 in non-desert
  //printf("lavaseed: %ld\n", lavaseed);
  if ( nextInt(&lavaseed, 8) != 0){ //nextInt(&lakeseed, 4) != 0  &&
    return 0;
  }

  if (nextInt(&lakeseed, 4) != 0 ){
    nextInt(&lakeseed, 16); //noise in X
    nextInt(&lakeseed, 16); //noise in Z
    int lakey = nextInt(&lakeseed, 256);
    if (lakey >= 63){
      return 0;
    }
  }

  //nextInt(&lakeseed, 16); //noise in X
  //nextInt(&lakeseed, 16); //noise in Z
  int adjx = nextInt(&lavaseed, 16); //noise in X
  int adjz = nextInt(&lavaseed, 16); //noise in Z
  
  //int lakey = nextInt(&lakeseed, 256);
  int temp = nextInt(&lavaseed, 256 - 8);
  int lavay = nextInt(&lavaseed, temp + 8);

  if (nextInt(&lavaseed, 10) != 0){
    return 0;
  }

  if (lavay > 80){
    //} && lakey < 63){
    //printf("lavay %d, lakey %d\n", lavay, lakey);
    *lx = x + adjx;
    *lz = z + adjz;
    return 1;
  }
  return 0;
}

int possible_lava(int64_t lower48, int x, int z, int* lx, int* lz){ //desert mode
  //printf("lower48: %ld\n", lower48);
  int64_t fakeseed = (lower48) ^ 0x5deece66dUL;
  long a = nextLong(&fakeseed) | 1;
  long b = nextLong(&fakeseed) | 1;
  fakeseed = ((long)x * a + (long)z * b) ^ lower48;
  fakeseed = fakeseed & 0xffffffffffff; //population seed set?
  //printf("population seed: %ld\n", fakeseed);
  int64_t lakeseed = (fakeseed + 10000) ^ 0x5deece66dUL;
  //printf("lakeseed: %ld\n", lakeseed);
  int64_t lavaseed = (fakeseed + 10000) ^ 0x5deece66dUL;//10001 in non-desert
  //printf("lavaseed: %ld\n", lavaseed);
  if ( nextInt(&lavaseed, 8) != 0){ //nextInt(&lakeseed, 4) != 0  &&
    return 0;
  }
  //nextInt(&lakeseed, 16); //noise in X
  //nextInt(&lakeseed, 16); //noise in Z
  int adjx = nextInt(&lavaseed, 16); //noise in X
  int adjz = nextInt(&lavaseed, 16); //noise in Z
  
  //int lakey = nextInt(&lakeseed, 256);
  int temp = nextInt(&lavaseed, 256 - 8);
  int lavay = nextInt(&lavaseed, temp + 8);

  if (nextInt(&lavaseed, 10) != 0){
    return 0;
  }

  if (lavay > 80){
    //} && lakey < 63){
    //printf("lavay %d, lakey %d\n", lavay, lakey);
    *lx = x + adjx;
    *lz = z + adjz;
    return 1;
  }
  return 0;
}

int lava_grid(int64_t lower48, int* lx, int* lz){
  int lava_count = 0;
  int cx,cz,temp;
  int storedx, storedz;
  for(cx = -4; cx < 10; cx++){
    for(cz = -4; cz < 10; cz++){
      temp = possible_lava(lower48, cx<<4, cz<<4, &storedx, &storedz);
      //temp = lava_non_desert(lower48, cx<<4, cz<<4, &storedx, &storedz);
      lava_count += temp;
      if (temp > 0){
        *lx = storedx;//in 1.15 no +8
        *lz = storedz;//in 1.15 no +8
        //printf("lava at (roughly) /tp @p %d 75 %d\n", *lx + 8, *lz +8);
      }
    }
  }
  if (lava_count < 2){
    return 0;
  }
  return 1;
}
//NOTES: I think 80, 8 Matthew said 4, 8 so test both, if I'm wrong then I might fool myself
//Because it's so probabilistic, I could probably tweak those lakey and lavay in order improve performance
//I want at least 75% of lava

int isDesert(int64_t seed, int x, int z, LayerStack* lp){
  Pos p;
  p.x = ((x >> 4) << 4); //in 1.14.4 but will need to adapt 1.15 
  p.z = ((z >> 4) << 4); //in 1.14.4 but will need to adapt 1.15
  int biome = getBiomeAtPos(lp, p) % 128;
  if (biome == desert){//} && biome != river && biome != swamp && !isOceanic(biome) && biome != desert_hills){
    return 1;
  }
  return 0;
}

int villageLocation(int64_t lower48){
  const StructureConfig sconf = VILLAGE_CONFIG;
  int valid;
  Pos p = getStructurePos(sconf, lower48, 0, 0, &valid);
  if (!valid || p.x > 144 || p.z > 144 ){
    return 0;
  }
  return 1;
}

int main(void){
  int64_t seed = 6720914721254245405; //-7436437356683320241;
  int64_t lower48, upper16;
  int foundSeed = 0;
  int foundBiome = 0;
  long seedCount = 0;
  initBiomes();
  LayerStack g;
  int mc = MC_1_14;
  setupGenerator(&g, mc);
  int lx,lz, tx, tz;
  int resetme = 1;
  int ohcrap = 0;
  while (resetme == 1){
    foundSeed = 0;
    foundBiome = 0;
    while (foundSeed == 0){
      seed = rand64();
      lower48 = seed >> 16;
      seedCount++;
      if (desertTemple(lower48, 100L*100L, &tx, &tz) == 1 && fortressCheck(lower48, 100L*100L) == 1 && lava_grid(lower48, &lx, &lz) == 1 && villageLocation(lower48)==1){
        foundSeed = 1;
      }
    }
    printf("found structure seed after %ld\n",seedCount);
    ohcrap = 0;
    while (foundBiome == 0 && ohcrap < 1500){
      ohcrap++;
      seed = rand64();
      upper16 = (seed >> 48) << 48;
      seed = lower48 | upper16;
      applySeed(&g, seed);
      Pos spwn = getSpawn(mc, &g, NULL, seed);
      const StructureConfig sconf = VILLAGE_CONFIG;
      int valid;
      Pos pv = getStructurePos(sconf, seed, 0, 0, &valid);
      if (isViableStructurePos(sconf.structType, mc, &g, seed, pv.x, pv.z) && isDesert(seed, lx+8, lz+8, &g)==1 && isDesert(seed, tx, tz, &g)==1 && (spwn.x > -96 && spwn.x <= 144 && spwn.z > -96 && spwn.z <= 144)){
        printf("desert biomes at %d,%d and %d,%d\n", lx,lz, tx, tz);
        foundBiome = 1;
        resetme = 0;
      }
    }
  }
  printf("Found kajodenn bugd seed: %ld after %ld seeds %d ohcrap\n", seed, seedCount, ohcrap);
  return 0;
}