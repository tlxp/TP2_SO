# Jogo de Batalha Naval

Este projeto é um jogo de Batalha Naval para dois jogadores implementado em C, utilizando pipes FIFO para comunicação entre processos.

**[English version](#english-version)**

---

## 📁 Estrutura do Projeto

- **`battleship.c`** – Lógica principal e implementação do jogo.
- **`Makefile`** – Automatiza a compilação e limpeza.
- **`player1_ships.txt`** – Armazena as coordenadas dos navios do Jogador 1.
- **`player2_ships.txt`** – Armazena as coordenadas dos navios do Jogador 2.
- **`fifo1`, `fifo2`** – Pipes nomeados para comunicação entre jogadores.
- **`README.md`** – Este documento.

---

## 🔧 Compilação

```bash
make
```

---

## ▶️ Execução

Execute duas instâncias do programa, uma para cada jogador:

### Jogador 1
```bash
./battleship p1 small|medium|big
```

### Jogador 2
```bash
./battleship p2 small|medium|big
```

**Modos de jogo:**
- **small**: Grelha 5x5, 2 navios de tamanho 2
- **medium**: Grelha 7x7, 3 navios de tamanho 3
- **big**: Grelha 10x10, 4 navios de tamanho 4

---

## 📄 Colocação de Navios

- **Os jogadores escolhem entre colocação manual ou aleatória dos navios.**
- **Manual**: Insira coordenadas (e.g., `A2 h` para horizontal ou `B3 v` para vertical).
- **Aleatória**: Os navios são colocados automaticamente e salvos nos respectivos ficheiros.

---

## 🧠 Mecânicas do Jogo

- **Os jogadores atacam alternadamente, especificando coordenadas (e.g., `A2`).**
- **Acertos (`X`) e falhas (`O`) são registrados na grelha de ataque.**
- **O jogo termina quando todos os navios de um jogador forem afundados.**
- **Comunicação via pipes FIFO (`fifo1`, `fifo2`) para coordenadas de ataque e resultados.**

---

## 📊 Saída

- **Grelhas exibidas no terminal com cores ANSI:**
  - **Azul `~`**: Água
  - **Verde `S`**: Navio (oculto na visão do oponente)
  - **Vermelho `X`**: Acerto
  - **Amarelo `O`**: Falha
- **Coordenadas dos navios salvas em `player1_ships.txt` ou `player2_ships.txt`.**
- **Estado do jogo (navios restantes) mostrado após cada turno.**

---

## 👥 Autores

- **[Seu Nome]** – [Seu ID ou Contato]

---

## 📌 Observações

- **Garanta permissões de escrita no diretório atual para pipes FIFO e ficheiros de navios.**
- **Execute o Jogador 1 primeiro para criar os FIFOs, depois o Jogador 2.**
- **Limpe os ficheiros gerados com:**
  ```bash
  make clean
  ```
- **Validação de entrada garante coordenadas válidas e evita sobreposições.**

---

## English Version

# Battleship Game

This project is a two-player Battleship game implemented in C, using FIFO pipes for inter-process communication.

**[Versão em Português](#jogo-de-batalha-naval)**

---

## 📁 Project Structure

- **`battleship.c`** – Main game logic and implementation.
- **`Makefile`** – Automates compilation and cleanup.
- **`player1_ships.txt`** – Stores Player 1's ship coordinates.
- **`player2_ships.txt`** – Stores Player 2's ship coordinates.
- **`fifo1`, `fifo2`** – Named pipes for communication between players.
- **`README.md`** – This document.

---

## 🔧 Compilation

```bash
make
```

---

## ▶️ Execution

Run two instances of the program, one for each player:

### Player 1
```bash
./battleship p1 small|medium|big
```

### Player 2
```bash
./battleship p2 small|medium|big
```

**Game modes:**
- **small**: 5x5 grid, 2 ships of size 2
- **medium**: 7x7 grid, 3 ships of size 3
- **big**: 10x10 grid, 4 ships of size 4

---

## 📄 Ship Placement

- **Players choose manual or random ship placement.**
- **Manual**: Enter coordinates (e.g., `A2 h` for horizontal or `B3 v` for vertical).
- **Random**: Ships are placed automatically and saved to respective files.

---

## 🧠 Game Mechanics

- **Players take turns attacking by specifying coordinates (e.g., `A2`).**
- **Hits (`X`) and misses (`O`) are tracked on the attack grid.**
- **Game ends when one player's ships are all sunk.**
- **Communication via FIFO pipes (`fifo1`, `fifo2`) for attack coordinates and results.**

---

## 📊 Output

- **Grids displayed in terminal with ANSI colors:**
  - **Blue `~`**: Water
  - **Green `S`**: Ship (hidden in opponent's view)
  - **Red `X`**: Hit
  - **Yellow `O`**: Miss
- **Ship coordinates saved in `player1_ships.txt` or `player2_ships.txt`.**
- **Game status (ships remaining) shown after each turn.**

---

## 👥 Authors

- **[Your Name]** – [Your ID or Contact]

---

## 📌 Notes

- **Ensure write permissions in the current directory for FIFO and ship files.**
- **Run Player 1 first to create FIFOs, then Player 2.**
- **Clean up generated files with:**
  ```bash
  make clean
  ```
- **Input validation ensures valid coordinates and prevents overlap.**