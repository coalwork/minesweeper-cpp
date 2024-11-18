#pragma once

#include <iostream>
#include "Grid.hpp"

std::string escape(std::string options) {
  return "\x1b" + options;
}

const std::string reset = escape("[0m");
const std::string resetCursor = escape("[H");
const std::string clear = escape("[2J");
const std::string saveCursor = escape("[s");
const std::string restoreCursor = escape("[u");
const std::string clearLine = escape("[2K");

std::string moveCursorEnd(int rows) {
  return escape("[" + std::to_string(rows + 2) + ";1H");
}

void drawGrid(Grid& grid) {
  const int rowIndicatorWidth = 3;

  const size_t numRows = grid.getRowCount();
  const size_t numCols = grid.getColumnCount();

  // only clear minefield display
  std::cout
    << "\x1b[" << numRows + 1 << ";" << numCols + rowIndicatorWidth + 1 << "H"
    << escape("[1J")
    << resetCursor;

  // draw column indices
    std::cout << std::setw(3) << "";
  for (size_t i = 1; i <= numCols; i++) {
    std::cout
      << escape("[" + std::to_string(31 + i / 10) + "m")
      << i % 10 << reset;
  }
  std::cout << std::endl;

  for (size_t i = 0; i < numRows; i++) {
    // draw row index
      std::cout
      << escape("[" + std::to_string(31 + (i + 1) / 10) + "m")
      << std::setw(2) << i + 1 << " " << reset;

    for (size_t j = 0; j < numCols; j++) {
      const Cell& cell = grid(i, j);

      if (cell.isFlagged()) {
        std::cout << escape("[30;103m") << "f" << reset;
        continue;
      }

      if (!cell.isRevealed()) {
        std::cout << "\u2591" << reset; // light shade
        continue;
      }

      if (cell.isMine()) {
        std::cout << escape("[97;101m") << "X" << reset;
      } else {
        int mineCount = cell.getMineCount();
        if (mineCount == 0) {
          std::cout << " ";
        } else {
          std::cout
            << escape("[" + std::to_string(89 + mineCount) + "m")
            << mineCount << reset;
        }
      }
    }

    std::cout << std::endl;
  }
}

void drawCellStatus(Cell &cell, int row, int column, int rows) {
  std::cout
    << moveCursorEnd(rows)
    << clearLine
    << escape("[90m")
    << "cell (" << row + 1 << ", " << column + 1 << "): ";

  if (cell.isFlagged()) {
    std::cout << "flagged";
  } else

  if (cell.isRevealed()) {
    std::cout
      << "nearby mines: " << cell.getMineCount();
  } else {
    std::cout << "hidden";
  }

  std::cout << restoreCursor;
}
