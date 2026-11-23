// Roda o gerador (WebAssembly) fora da tela principal, para o mapa não travar.
importScripts('biomas.js');

let ex = null;
let iniciando = null;
const TAM_TILE = 128; // células por lado de cada pedaço do mapa

async function iniciar() {
  const bytes = await (await fetch('gerador.wasm')).arrayBuffer();
  // O módulo só pede funções de arquivo da libc, que ele nunca usa aqui.
  const vazio = new Proxy({}, { get: () => () => 0 });
  const { instance } = await WebAssembly.instantiate(bytes, { wasi_snapshot_preview1: vazio });
  ex = instance.exports;
  ex._initialize();
}

function definirSeed(texto) {
  const b = BigInt.asUintN(64, BigInt(texto));
  ex.bs_seed(Number(b & 0xffffffffn), Number(b >> 32n));
}

function estruturas(tipo, x0, z0, x1, z1, max) {
  const p = ex.bs_alocar(12 * max);
  const n = ex.bs_estruturas(tipo, x0, z0, x1, z1, p, max);
  const dados = n > 0 ? Array.from(new Int32Array(ex.memory.buffer, p, n * 3)) : [];
  ex.bs_liberar(p);
  const lista = [];
  for (let i = 0; i < dados.length; i += 3) lista.push({ tipo, x: dados[i], z: dados[i + 1], sub: dados[i + 2] });
  return lista;
}

const CORES = new Uint8Array(256 * 3).fill(60);
for (const [id, [, r, g, b]] of Object.entries(BIOMAS)) { CORES[id * 3] = r; CORES[id * 3 + 1] = g; CORES[id * 3 + 2] = b; }

function tile(escala, tx, tz) {
  const n = TAM_TILE * TAM_TILE;
  const p = ex.bs_alocar(4 * n);
  const erro = ex.bs_biomas(escala, tx * TAM_TILE, tz * TAM_TILE, TAM_TILE, TAM_TILE, p);
  const ids = new Int32Array(ex.memory.buffer, p, n);
  const rgba = new Uint8ClampedArray(n * 4);
  for (let i = 0; i < n; i++) {
    const id = erro ? 0 : ids[i] & 255;
    rgba[i * 4] = CORES[id * 3]; rgba[i * 4 + 1] = CORES[id * 3 + 1]; rgba[i * 4 + 2] = CORES[id * 3 + 2]; rgba[i * 4 + 3] = 255;
  }
  ex.bs_liberar(p);
  return rgba;
}

onmessage = async e => {
  const m = e.data;
  try {
    if (!ex) await (iniciando || (iniciando = iniciar()));
    if (m.tipo === 'seed') {
      definirSeed(m.seed);
      postMessage({ id: m.id, ok: true });
    } else if (m.tipo === 'estruturas') {
      const lista = [];
      for (const t of m.tipos) lista.push(...estruturas(t, m.x0, m.z0, m.x1, m.z1, m.max || 4096));
      postMessage({ id: m.id, ok: true, lista });
    } else if (m.tipo === 'tile') {
      const rgba = tile(m.escala, m.tx, m.tz);
      postMessage({ id: m.id, ok: true, rgba }, [rgba.buffer]);
    } else if (m.tipo === 'bioma') {
      const id = ex.bs_bioma(m.x, m.z);
      postMessage({ id: m.id, ok: true, bioma: BIOMAS[id] ? BIOMAS[id][0] : '' });
    }
  } catch (erro) {
    postMessage({ id: m.id, ok: false, erro: String(erro) });
  }
};
