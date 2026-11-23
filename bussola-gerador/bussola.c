// Gerador da Bússola: posição das estruturas do Minecraft Bedrock e mapa de biomas.
// Compila para WebAssembly (build.sh). Os biomas vêm do cubiomes (MIT): desde a 1.18,
// Java e Bedrock geram os mesmos biomas com a mesma seed.
// As estruturas seguem a regra do Bedrock: para cada região, um Mersenne Twister
// (MT19937) recebe regiãoX*341873128712 + regiãoZ*132897987541 + seed + sal (32 bits
// de baixo) e sorteia o chunk. Sal, tamanho da região e tipo de sorteio de cada
// estrutura são os mesmos publicados pelos projetos abertos SeedFinder (MIT) e
// MCBE-seedcracker. As câmaras de desafio usam a regra do Java, como no Bedrock.
#include "cubiomes/finders.h"
#include "cubiomes/generator.h"
#include <stdint.h>
#include <stdlib.h>

#ifdef __wasm__
#define EXPORTA(nome) __attribute__((export_name(nome)))
#else
#define EXPORTA(nome)
#endif

// ---------- Mersenne Twister padrão (MT19937) ----------
typedef struct { uint32_t s[624]; int i; } MT;

static void mt_semente(MT *m, uint32_t semente) {
    m->s[0] = semente;
    for (int i = 1; i < 624; i++) m->s[i] = 1812433253u * (m->s[i - 1] ^ (m->s[i - 1] >> 30)) + (uint32_t)i;
    m->i = 624;
}
static uint32_t mt_prox(MT *m) {
    if (m->i >= 624) {
        for (int k = 0; k < 624; k++) {
            uint32_t y = (m->s[k] & 0x80000000u) | (m->s[(k + 1) % 624] & 0x7fffffffu);
            m->s[k] = m->s[(k + 397) % 624] ^ (y >> 1) ^ ((y & 1u) ? 0x9908b0dfu : 0u);
        }
        m->i = 0;
    }
    uint32_t y = m->s[m->i++];
    y ^= y >> 11;
    y ^= (y << 7) & 0x9d2c5680u;
    y ^= (y << 15) & 0xefc60000u;
    return y ^ (y >> 18);
}

// ---------- Tipos que o app mostra ----------
enum {
    T_VILA = 1, T_CIDADE_ANCESTRAL, T_CAMARA, T_MANSAO, T_MONUMENTO, T_POSTO,
    T_TEMPLO, T_NAUFRAGIO, T_PORTAL, T_TESOURO, T_COMPLEXO_NETHER, T_PORTAL_NETHER,
};
// Subtipo do T_TEMPLO: os quatro usam a mesma grade e o bioma decide qual nasce.
enum { TEMPLO_DESERTO = 1, TEMPLO_SELVA = 2, CABANA_BRUXA = 3, IGLU = 4 };

typedef struct { uint32_t sal; int regiao; int alcance; int triangular; } Grade;

static int grade(int tipo, Grade *g) {
    switch (tipo) {
    case T_VILA:            *g = (Grade){ 10387312u, 34, 26, 1 }; return 1;
    case T_CIDADE_ANCESTRAL:*g = (Grade){ 20083232u, 24, 16, 1 }; return 1;
    case T_MANSAO:          *g = (Grade){ 10387319u, 80, 60, 1 }; return 1;
    case T_MONUMENTO:       *g = (Grade){ 10387313u, 32, 27, 1 }; return 1;
    case T_POSTO:           *g = (Grade){ 165745296u, 80, 56, 1 }; return 1;
    case T_TESOURO:         *g = (Grade){ 16842397u, 4, 2, 1 }; return 1;
    case T_TEMPLO:          *g = (Grade){ 14357617u, 32, 24, 0 }; return 1;
    case T_NAUFRAGIO:       *g = (Grade){ 165745295u, 24, 20, 0 }; return 1;
    case T_PORTAL:          *g = (Grade){ 40552231u, 40, 25, 0 }; return 1;
    case T_COMPLEXO_NETHER: *g = (Grade){ 30084232u, 30, 26, 0 }; return 1;
    case T_PORTAL_NETHER:   *g = (Grade){ 40552231u, 25, 15, 0 }; return 1;
    default: return 0;
    }
}

static uint64_t semente = 0;
static Generator gen;
static int pronto = 0;

// Chunk da estrutura dentro da região, pela regra do Bedrock.
static Pos posBedrock(const Grade *g, int rx, int rz) {
    MT m;
    uint64_t mistura = (uint64_t)(int64_t)rx * UINT64_C(341873128712) + (uint64_t)(int64_t)rz * UINT64_C(132897987541) + semente + g->sal;
    mt_semente(&m, (uint32_t)mistura);
    int cx, cz;
    if (g->triangular) {
        int x1 = (int)(mt_prox(&m) % (uint32_t)g->alcance);
        int x2 = (int)(mt_prox(&m) % (uint32_t)g->alcance);
        int z1 = (int)(mt_prox(&m) % (uint32_t)g->alcance);
        int z2 = (int)(mt_prox(&m) % (uint32_t)g->alcance);
        cx = (x1 + x2) / 2;
        cz = (z1 + z2) / 2;
    } else {
        cx = (int)(mt_prox(&m) % (uint32_t)g->alcance);
        cz = (int)(mt_prox(&m) % (uint32_t)g->alcance);
    }
    Pos p = { (rx * g->regiao + cx) * 16 + 8, (rz * g->regiao + cz) * 16 + 8 };
    return p;
}

// O Bedrock confere o bioma no centro do chunk da própria estrutura.
static int biomaDaCelula(int x, int z, int yCelula) {
    return getBiomeAt(&gen, 4, (x >> 4) * 4 + 2, yCelula, (z >> 4) * 4 + 2);
}
// Sem taiga nevada: na seed 6666, conferida dentro do jogo, as tentativas nesse bioma não viram vila.
// Gabarito (seed 6666, até 1600 blocos de 0,0): -168,56  744,-344  -312,-1000  744,-872  248,1144  -312,1208  1256,-328  56,-1480.
static int ehVilaBioma(int b) {
    return b == desert || b == plains || b == meadow || b == savanna || b == snowy_plains ||
           b == taiga || b == sunflower_plains;
}

// Devolve o subtipo (>= 1) se a estrutura nasce ali, ou 0 se o bioma não deixa.
static int viavel(int tipo, int x, int z) {
    int b;
    switch (tipo) {
    case T_VILA: return ehVilaBioma(biomaDaCelula(x, z, 319 >> 2));
    case T_CIDADE_ANCESTRAL: return isViableStructurePos(Ancient_City, &gen, x, z, 0) ? 1 : 0;
    case T_MANSAO: b = biomaDaCelula(x, z, 319 >> 2); return b == dark_forest || b == pale_garden;
    case T_MONUMENTO: return isDeepOcean(biomaDaCelula(x, z, 319 >> 2));
    case T_POSTO: return isViableFeatureBiome(MC_NEWEST, Outpost, biomaDaCelula(x, z, 319 >> 2));
    case T_TESOURO: return isViableFeatureBiome(MC_NEWEST, Treasure, biomaDaCelula(x, z, 319 >> 2));
    case T_NAUFRAGIO: b = biomaDaCelula(x, z, 319 >> 2); return b == beach || b == snowy_beach || isOceanic(b);
    case T_TEMPLO:
        b = biomaDaCelula(x, z, 319 >> 2);
        if (b == desert) return TEMPLO_DESERTO;
        if (b == jungle || b == bamboo_jungle) return TEMPLO_SELVA;
        if (b == swamp || b == mangrove_swamp) return CABANA_BRUXA;
        if (b == snowy_plains || b == snowy_taiga || b == snowy_slopes) return IGLU;
        return 0;
    case T_PORTAL: case T_COMPLEXO_NETHER: case T_PORTAL_NETHER: return 1;
    default: return 0;
    }
}

static int divChao(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

EXPORTA("bs_seed") void bs_seed(uint32_t lo, uint32_t hi) {
    semente = ((uint64_t)hi << 32) | lo;
    setupGenerator(&gen, MC_NEWEST, 0);
    applySeed(&gen, DIM_OVERWORLD, semente);
    pronto = 1;
}

// Estruturas dentro do retângulo [x0,x1]×[z0,z1] (blocos). Escreve trincas (x, z, subtipo).
EXPORTA("bs_estruturas") int bs_estruturas(int tipo, int x0, int z0, int x1, int z1, int32_t *saida, int max) {
    if (!pronto) return -1;
    int n = 0;
    if (tipo == T_CAMARA) {
        StructureConfig sc;
        if (!getStructureConfig(Trial_Chambers, MC_NEWEST, &sc)) return 0;
        int tam = sc.regionSize * 16;
        for (int rz = divChao(z0, tam) - 1; rz <= divChao(z1, tam) + 1; rz++)
            for (int rx = divChao(x0, tam) - 1; rx <= divChao(x1, tam) + 1; rx++) {
                Pos p;
                if (!getStructurePos(Trial_Chambers, MC_NEWEST, semente, rx, rz, &p)) continue;
                p.x += 8; p.z += 8;
                if (p.x < x0 || p.x > x1 || p.z < z0 || p.z > z1) continue;
                if (!isViableStructurePos(Trial_Chambers, &gen, p.x, p.z, 0)) continue;
                if (n >= max) return n;
                saida[n * 3] = p.x; saida[n * 3 + 1] = p.z; saida[n * 3 + 2] = 1; n++;
            }
        return n;
    }
    Grade g;
    if (!grade(tipo, &g)) return 0;
    int tam = g.regiao * 16;
    for (int rz = divChao(z0, tam) - 1; rz <= divChao(z1, tam) + 1; rz++)
        for (int rx = divChao(x0, tam) - 1; rx <= divChao(x1, tam) + 1; rx++) {
            Pos p = posBedrock(&g, rx, rz);
            if (p.x < x0 || p.x > x1 || p.z < z0 || p.z > z1) continue;
            int sub = viavel(tipo, p.x, p.z);
            if (!sub) continue;
            if (n >= max) return n;
            saida[n * 3] = p.x; saida[n * 3 + 1] = p.z; saida[n * 3 + 2] = sub; n++;
        }
    return n;
}

// Bioma de um bloco da superfície.
EXPORTA("bs_bioma") int bs_bioma(int x, int z) {
    if (!pronto) return -1;
    return getBiomeAt(&gen, 4, x >> 2, 63 >> 2, z >> 2);
}

// Grade de biomas (largura × altura) com 1 célula = escala blocos, a partir de (x0, z0) em células.
EXPORTA("bs_biomas") int bs_biomas(int escala, int x0, int z0, int largura, int altura, int32_t *saida) {
    if (!pronto) return -1;
    Range r = { escala, x0, z0, largura, altura, escala == 1 ? 63 : 63 / escala, 1 };
    int *cache = allocCache(&gen, r);
    if (!cache) return -2;
    int erro = genBiomes(&gen, cache, r);
    if (!erro) for (int i = 0; i < largura * altura; i++) saida[i] = cache[i];
    free(cache);
    return erro;
}

EXPORTA("bs_alocar") void *bs_alocar(int bytes) { return malloc((size_t)bytes); }
EXPORTA("bs_liberar") void bs_liberar(void *p) { free(p); }
