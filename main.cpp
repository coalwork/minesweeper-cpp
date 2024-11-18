#include <iostream>
#include <iomanip>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <termios.h>
#include <limits>

#include "Grid.hpp"
#include "display.hpp"
#include "terminal.hpp"

enum class Direction {
  UP = 'k',
  DOWN = 'j',
  RIGHT = 'l',
  LEFT = 'h'
};

int main(int argc, char *argv[]) {
  // disable pseudo-random generation
  time_t seed = time(nullptr);
  srand(seed);

  /* old default settings
  const size_t rows = 24;
  const size_t cols = 64;
  const int mines = rows * cols / 10;
  */

  const size_t rows = std::stoi(argv[1]);
  const size_t cols = std::stoi(argv[2]);
  const int mines = std::stoi(argv[3]);

  if (rows < 4 || cols < 4) {
    std::cerr << "rows and columns must be at least 4" << std::endl;
    return 1;
  }

  if (mines < 1 || mines > rows * cols) {
    std::cerr << "mine count must be between 1 and rows * cols" << std::endl;
    return 1;
  }

  Grid grid(rows, cols);

  std::cout << clear;
  drawGrid(grid);

  // controls
  std::cout
    << std::endl
    << "hjkl: left, down, up, right\n"
    << "yuio: quick movement\n"
    << "d: dig\n"
    << "f: flag\n"
    << "q: quit";

  // move to middle cell
  {
    int r = rows / 2 + 1;
    int c = cols / 2 + 1;

    std::cout << "\x1b[" << r + 1 << ";" << c + 3 << "H";

    // status for middle cell
    std::cout << saveCursor;
    drawCellStatus(grid(r, c), r, c, rows);
    std::cout << restoreCursor;
  }

  bool win = true;
  bool quit = false;
  char c;
  initTermios(0);
  while (true) {
    std::cin.read(&c, 1);

    // cursor movement
    if (c == 'h' || c == 'j' || c == 'k' || c == 'l') {
      int row, column;
      parsePosition(getCursorPosition(), row, column);
      
      Direction dir = static_cast<Direction>(c);

      // directional cases
      switch (dir) {
        case Direction::UP:
          if (row == 2) { break; }
          std::cout << escape("[A");
          break;
        case Direction::DOWN:
          if (row == rows + 1) { break; }
          std::cout << escape("[B");
          break;
        case Direction::RIGHT:
          if (column == cols + 3) { break; }
          std::cout << escape("[C");
          break;
        case Direction::LEFT:
          if (column == 4) { break; }
          std::cout << escape("[D");
          break;
      }

      parsePosition(getCursorPosition(), row, column);

      Cell &cell = grid(row - 2, column - 4);

      std::cout << saveCursor;
      drawCellStatus(cell, row - 2, column - 4, rows);
      std::cout << restoreCursor;

      continue;
    }

    // quick movement - move to next revealed cell
    if (c == 'o') { // left
      int row, column;
      parsePosition(getCursorPosition(), row, column);
      
      row -= 2;
      column -= 4;

      if (column >= cols - 1) { continue; }

      int moveCount = -1;
      for (int i = column; i < cols && grid(row, i).isRevealed(); i++, moveCount++) {}
      if (moveCount == -1) { moveCount = 1; }

      std::cout << escape("[" + std::to_string(moveCount) + "C");

      continue;
    }
    if (c == 'y') { // right
      int row, column;
      parsePosition(getCursorPosition(), row, column);
      
      row -= 2;
      column -= 4;

      if (column <= 0) { continue; }

      int moveCount = -1;
      for (int i = column; i >= 0 && grid(row, i).isRevealed(); i--, moveCount++) {}
      if (moveCount == -1) { moveCount = 1; }

      std::cout << escape("[" + std::to_string(moveCount) + "D");

      continue;
    }
    if (c == 'u') { // up
      int row, column;
      parsePosition(getCursorPosition(), row, column);
      
      row -= 2;
      column -= 4;

      if (row >= rows - 1) { continue; }

      int moveCount = -1;
      for (int i = row; i < rows && grid(i, column).isRevealed(); i++, moveCount++) {}
      if (moveCount == -1) { moveCount = 1; }

      std::cout << escape("[" + std::to_string(moveCount) + "B");

      continue;
    }
    if (c == 'i') { // down
      int row, column;
      parsePosition(getCursorPosition(), row, column);
      
      row -= 2;
      column -= 4;

      if (row <= 0) { continue; }

      int moveCount = -1;
      for (int i = row; i >= 0 && grid(i, column).isRevealed(); i--, moveCount++) {}
      if (moveCount == -1) { moveCount = 1; }

      std::cout << escape("[" + std::to_string(moveCount) + "A");

      continue;
    }

    // dig
    if (c == 'd') {
      int row, column;
      parsePosition(getCursorPosition(), row, column);

      if (!grid.getInitializationState()) { grid.generate(mines, row - 2, column - 4); }

      Cell &cell = grid(row - 2, column - 4);

      if (cell.isFlagged()) { continue; }

      // if revealed cell is dug, and neighboring flags equals
      // mine count, reveal all neighbors
      if (cell.isRevealed() && grid.countFlaggedNeighbors(row - 2, column - 4) == cell.getMineCount()) {
        for (int i = 0; i <= 8; i++) {
          if (i == 4) { continue; } // skip self
          bool res = grid.reveal(row - 2 + i / 3 - 1, column - 4 + i % 3 - 1);

          if (res) {
            grid.revealAllMines();
            win = false;
          }
        }

        std::cout << saveCursor;
        drawGrid(grid);
        std::cout << restoreCursor;

        if (grid.isWin() || !win) { break; }

        continue;
      }

      bool res = grid.reveal(row - 2, column - 4);

      if (!res) {
        std::cout << saveCursor;
        drawCellStatus(cell, row - 2, column - 4, rows);
        std::cout << restoreCursor;
      } else {
        grid.revealAllMines();
        win = false;
      }

      std::cout << saveCursor;
      drawGrid(grid);
      std::cout << restoreCursor;

      if (grid.isWin() || !win) { break; }

      continue;
    }

    // flag
    if (c == 'f') {
      int row, column;
      parsePosition(getCursorPosition(), row, column);

      Cell &cell = grid(row - 2, column - 4);

      if (cell.isRevealed()) { continue; }

      cell.toggleFlag();

      std::cout << saveCursor;
      drawGrid(grid);
      drawCellStatus(cell, row - 2, column - 4, rows);
      std::cout << restoreCursor;

      continue;
    }

    // quit
    if (c == 'q') {
      std::cout
        << saveCursor
        << escape("[" + std::to_string(rows + 2) + ";1H")
        << escape("[1;31m")
        << "are you sure you want to quit? (y/N): "
        << reset;

      char option;
      std::cin >> option;

      // escape sequences will break prompt
      if (option != '\x1b') {
        std::cout << option;
      } else {
        std::cout << "invalid input received, press <enter> to continue";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      }

      if (option == 'y' || option == 'Y') {
        quit = true;
        break;
      }

      std::cout << clearLine << std::endl << restoreCursor;

      // restore cell status after quit prompt
      int row, column;
      parsePosition(getCursorPosition(), row, column);

      std::cout << saveCursor;
      drawCellStatus(grid(row - 2, column - 4), row - 2, column - 4, rows);
      std::cout << restoreCursor;

      continue;
    }
  }

  std::cout << "\x1b[" << rows + 8 << ";1H"; // +8 to move past controls
  std::cout << (win ? quit ? "game ended" : "you win" : "you lose")<< std::endl;
  resetTermios();

  return 0;
}
