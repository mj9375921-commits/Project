# Notas para o Claude

- O dono do repositório fala português do Brasil. Responda em português simples.
- Quando ele pedir um "app", faça um APP de celular que instala na tela inicial (PWA com manifest,
  ícones e service worker), não só um site. Todo mundo do server abre pelo celular.
- `docs/` é o app Coords (coordenadas do server de Minecraft). O GitHub Pages publica a pasta `docs/`.
  Leia `docs/DESIGN.md` antes de mudar o visual.
- `docs/server/` é o app ResenhaCraft: mostra se o server (Bedrock, Aternos) está online. Usa as APIs
  api.mcstatus.io e api.mcsrvstat.us. Bedrock não informa os nomes dos jogadores, só quantos estão on.
- Se mudar a lista de arquivos em `docs/sw.js`, aumente o número em `CACHE`.
- `src/` é um projeto Remotion (vídeo). Checagem: `npm run lint`.
