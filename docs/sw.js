// Guarda o app no aparelho para ele abrir sem internet.
// Mude o número da versão quando trocar a lista de arquivos.
const CACHE = 'coords-v3';
const ARQUIVOS = [
  './',
  './index.html',
  './manifest.webmanifest',
  './fonts/jersey10-latin.woff2',
  './icons/icon.svg',
  './icons/icon-192.png',
  './icons/icon-512.png',
  './icons/apple-touch-icon.png',
  './server/',
  './server/manifest.webmanifest',
  './server/icons/icon.svg',
  './server/icons/icon-192.png',
  './server/icons/icon-512.png',
  './server/icons/apple-touch-icon.png',
  './embaixada/',
  './embaixada/manifest.webmanifest',
  './embaixada/icons/icon.svg',
  './embaixada/icons/icon-192.png',
  './embaixada/icons/icon-512.png',
  './embaixada/icons/apple-touch-icon.png',
];

self.addEventListener('install', e => {
  e.waitUntil(caches.open(CACHE).then(c => c.addAll(ARQUIVOS)).then(() => self.skipWaiting()));
});

self.addEventListener('activate', e => {
  e.waitUntil(
    caches.keys()
      .then(nomes => Promise.all(nomes.filter(n => n !== CACHE).map(n => caches.delete(n))))
      .then(() => self.clients.claim()),
  );
});

// Responde com a cópia salva e atualiza a cópia pela rede para a próxima vez.
self.addEventListener('fetch', e => {
  const req = e.request;
  if (req.method !== 'GET' || new URL(req.url).origin !== self.location.origin) return;
  e.respondWith(caches.open(CACHE).then(async cache => {
    const salvo = await cache.match(req, { ignoreSearch: true });
    const rede = fetch(req).then(res => {
      if (res.ok) cache.put(req, res.clone());
      return res;
    });
    if (salvo) {
      e.waitUntil(rede.catch(() => {}));
      return salvo;
    }
    return rede;
  }));
});
