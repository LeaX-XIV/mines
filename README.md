# mines

mines is a simple Minesweeper for the console written in C.  

## Build

### Linux

```console
make
```

### Windows

Use [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) or [Make for Windows](https://gnuwin32.sourceforge.net/packages/make.htm), and run

```console
make
```

## Usage

```console
./mines [<difficulty>] [<size> [<bombs>]]
```

- `difficulty: easy|medium|hard`
- `size: full|(<rows> [<cols>])`
- `rows: int > 0`
- `cols: int > 0`
- `bombs: int > 0`

| Difficulty     | Settings         |
| :------------- | :--------------- |
| easy (Default) | 10x10,  10 bombs |
| medium         | 15x15,  40 bombs |
| hard           | 15x30, 100 bombs |

By specifying `full`, the grid becomes the size of the current terminal, and the number of bombs is proportional with the chosen difficulty.

### In-game commands

| Key          | Effect                          |
| :----------- | :------------------------------ |
| **q**        | Quit the game                   |
| **r**        | Restart                         |
| arrow keys   | Move the cursor around the grid |
| space, enter | Reveal cell at cursor position  |
| **f**        | Toggle flag at cursor position  |
