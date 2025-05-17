Queens Generation Tool
================
C++ tool for automatically generating Queens games.

# Installation

## Linux & MacOS
Use Make:
```
git clone https://github.com/tux76/queens.git
cd queens
make all
```

## Windows
Use MinGW:
```
git clone https://github.com/tux76/queens.git
cd queens
g++ queens.cpp -o queens.exe
```

# Usage
```
$ ./queens generate [BOARD_SIZE] [OPTIONS]
```

## Example
```
$ ./queens generate 8 --hide-queens
```
Generates a game with a width and height of 8 without showing the locations of the queens.

# Features
- Automatically generates queens games; prints a number for each color
```
$ ./queens generate 8
...
Completed Board: 
0 0 2 2 2 7 7 7 
0 0 7 7 7 7 7 7 
0 0 5 5 5 5 7 7 
5 0 5 4 4 7 7 7 
5 5 5 5 4 4 4 7 
3 3 3 4 4 4 6 7 
3 1 3 3 3 6 6 6 
3 3 3 3 3 6 6 6 
```

- Generated games have ***one and only one*** solution, meaning they can be solved like any official Queens game.

- Supports CSV output with `-s` option:
```
$ ./queens generate 8 -s output.csv
```
*Note: The last row of the CSV file represents the X position of queens based on Y index.*

- Colors queens yellow using ANSI escape codes. Disable with `--hide-queens`.

# Algorithm
- **Generate queens**: Generate a possible arrangement of queens. Assign a color to each.
- **Make base**: Choose a base color and fill all spaces (except for queens) with it. This starting position only has one solution.
- **Spread**: Find places where the base color can be replaced with another color while the game maintains only one solution. Randomly pick one of these and apply it.
- **Repeat**: Repeat last step *n* number of times. *n* is arbitrary and, for this program, is set to the *board width* squared.

# Improvements & Limitations
This project has much potential to grow, and any contributions are welcome!

- ⭐ Needs a user-friendly interface both for generating and playing generated games. *For now they have to be drawn by hand for any actual solving*
- ⭐ Option for multiple generations of saved data.
- Could have a GitHub-hosted database of pre-generated games and an online website for playing them on the go.
- Weighting or limiting the randomness for less desirable boards (e.g. one with colors occupying only 1 square).
- Limited by the "careful stepping" of each color change needing to create a game with a single solution. It is conceivable that some games may not be reachable with this method. *(minor)*

# About
- Author: tux76 (Jonas Blackwood)
- Date: 5-16-2025

# License
GNU GPL 3
