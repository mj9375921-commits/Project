# Coords: design

App de celular para guardar e converter coordenadas do server de Minecraft.
Abre pelo navegador e instala como app (PWA). Funciona sem internet.

Ideia visual: noite no mundo de blocos, com luz de tocha. Fundo escuro azulado,
destaque cor de tocha e uma cor fixa para cada dimensão. Bordas retas, botões
com relevo de bloco e números em fonte pixel.

## Tipografia

- Títulos e números: **Jersey 10** (fonte pixel, arquivo em `fonts/`, licença OFL). Só peso 400, porque ela já é grossa.
  A Pixelify Sans foi testada e descartada: nela o Z parece 2 e o 5 parece S.
- Texto: fonte do sistema (Roboto no Android, SF no iPhone). Pesos 400 e 600.
- Escala (px): 12 / 14 / 16 / 20 / 28 / 40. A fonte pixel desenha as letras menores, por isso usa os tamanhos maiores.
  - 40: nome do app e coordenadas grandes (pixel).
  - 28: títulos e coordenadas da lista e dos campos (pixel).
  - 20: rótulos em pixel (OVERWORLD, NETHER e o X Y Z pequeno).
  - 16: texto e botões.
  - 14: dicas, rótulos de campo e conversão.
  - 12: abas e rótulos pequenos.
- Campos de texto têm 16 px ou mais, para o iPhone não dar zoom.

## Cores

Só tema escuro, de propósito: o app é usado durante o jogo.

| Token | Valor | Uso |
|---|---|---|
| `--bg` | `#10151F` | fundo |
| `--surface` | `#182030` | barra de abas e folhas |
| `--surface-2` | `#212B3F` | campos e botões |
| `--line` | `#2A3448` | divisórias |
| `--edge` | `#6E7A95` | borda de campos e botões |
| `--text` | `#EEF2F8` | texto |
| `--text-dim` | `#A3AFC4` | texto secundário |
| `--torch` | `#FFB938` | destaque, botão principal e foco |
| `--on-torch` | `#241600` | texto no botão principal |
| `--ow` | `#74C94F` | Overworld (grama) e sucesso |
| `--nether` | `#FF6B57` | Nether |
| `--end` | `#C7A2FF` | End |
| `--danger` | `#FF7A70` | erro e apagar |

## Espaçamento

Escala: 4 / 8 / 16 / 24 / 40 / 64 px (`--sp-*`). Margem, padding e gap usam só esses valores.

## Componentes

- **Botão principal:** fundo tocha, texto escuro, relevo de bloco. Altura mínima 44 px.
- **Botão comum:** fundo `--surface-2`, borda `--edge`.
- **Estados dos botões:** apertado desce 1 px e perde o relevo. Foco: contorno de 3 px cor de tocha. Desativado: 50% de opacidade.
- **Campo:** fundo `--surface-2`, borda de 2 px `--edge`. No foco, a borda fica cor de tocha.
- **Campo de número:** tem um botão ± ao lado, porque o teclado numérico do iPhone não tem sinal de menos.
- **Lista de locais:** linhas com divisória, sem cards. Cada linha tem um quadrado com o ícone na cor da dimensão, o nome, a dimensão e o tipo, as coordenadas e a conversão.
- **Folha:** sobe de baixo, com cantos de cima de 8 px e fundo `--surface`.
- **Navegação:** barra de abas fixa embaixo (Conversor e Locais). A aba ativa fica cor de tocha, com um traço em cima.
- **Aviso:** fundo claro e texto escuro, acima da barra de abas, com ação opcional (Desfazer, Ver).
- **Ícones:** desenhados em pixel numa grade de 10×10, sem emoji.

## Layout

- Uma coluna, com largura máxima de 560 px, centralizada.
- Margem lateral mínima de 16 px, respeitando a área segura do iPhone.
- Conversor: grade com 3 linhas (X, Y, Z) e 2 colunas (Overworld e Nether).
- Celular primeiro. Em telas largas, o conteúdo só cresce até 560 px.
