#include "cubiomes/util.h"
#include "cubiomes/biomes.h"
#include <stdio.h>
int main(void) {
  unsigned char c[256][3]; initBiomeColors(c);
  printf("// Cores e nomes dos biomas (cores do cubiomes, MIT). Gerado por bussola-gerador/paleta.c.\nconst BIOMAS = {\n");
  for (int id = 0; id < 256; id++) {
    if (!biomeExists(MC_NEWEST, id)) continue;
    printf("  %d: ['%s', %d, %d, %d],\n", id, biome2str(MC_NEWEST, id), c[id][0], c[id][1], c[id][2]);
  }
  printf("};\n");
}
