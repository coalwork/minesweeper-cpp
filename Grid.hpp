#pragma once

#include <stdexcept>
#include <iostream>

#include "Cell.hpp"

class Grid {
  size_t nRows;
  size_t nCols;
  int mineCount;
  int revealCount = 0;
  bool isInitialized = false;
  Cell* grid;

  Cell* getNthNeighbor(int n, size_t r, size_t c) {
    if (n < 0 || n > 7) { throw std::invalid_argument("n must be between 0 and 7 inclusive"); }
    if (n >= 4) { n++; }

    int row = r + n / 3 - 1;
    int col = c + n % 3 - 1;

    if (row < 0 || row >= nRows || col < 0 || col >= nCols) { return nullptr; }
    return &(*this)(row, col);
  }

  public:
  bool isWin() const { return revealCount == nRows * nCols - mineCount; }
  bool getInitializationState() const { return isInitialized; }
  size_t getRowCount() const { return nRows; }
  size_t getColumnCount() const { return nCols; }
  Grid(size_t nRows, size_t nCols) : nRows(nRows), nCols(nCols), mineCount(0) {
    if (nRows <= 0 || nCols <= 0) {
      throw std::invalid_argument("nRows and nCols must be positive");
    }

    grid = new Cell[nRows * nCols];

    for (size_t i = 0; i < nRows; i++) {
      for (size_t j = 0; j < nCols; j++) {
        (*this)(i, j) = Cell();
      }
    }
  }
  void generate(int mineCount, size_t preservedRow, size_t preservedColumn) {
    if (mineCount > nRows * nCols) {
      throw std::invalid_argument("mineCount must be less than or equal to nRows * nCols");
    }
    if (preservedRow >= nRows || preservedColumn >= nCols) {
      throw std::out_of_range("preserveRow and preserveColumn must be within bounds");
    }

    this->mineCount = mineCount;

    for (size_t i = 0; i < mineCount;) {
      size_t r = rand() % nRows;
      size_t c = rand() % nCols;

      // do not encapsulate mines
      bool surroundedByMines = true;
      for (int j = 0; j <= 7; j++) {
        Cell* cell = getNthNeighbor(j, r, c);
        if (cell == nullptr) { continue; }
        surroundedByMines &= cell->isMine();
      }
      if (surroundedByMines) { continue; }

      // do not place mine on initial cell
      if (r == preservedRow && c == preservedColumn) { continue; }
      bool isNeighborOfPreservedCell = false;
      for (int j = 0; j <= 7; j++) {
        Cell* cell = getNthNeighbor(j, preservedRow, preservedColumn);
        if (cell == nullptr) { continue; }
        isNeighborOfPreservedCell |= cell == &(*this)(r, c);
      }
      if (isNeighborOfPreservedCell) { continue; }

      if (!(*this)(r, c).isMine()) {
        (*this)(r, c).setMine();

        for (int j = 0; j <= 7; j++) {
          Cell* cell = getNthNeighbor(j, r, c);
          if (cell != nullptr) { cell->incrementMineCount(); }
        }

        i++;
      }
    }

    isInitialized = true;
  }
  ~Grid() { delete[] grid; }
  Cell& operator() (size_t i, size_t j) const {
    if (i < 0 || i >= nRows || j < 0 || j >= nCols) {
      throw std::out_of_range("i and j must be within bounds");
    }
    return grid[i * nCols + j];
  }
  bool reveal(size_t r, size_t c) {
    if (r < 0 || r >= nRows || c < 0 || c >= nCols) { return false; }

    Cell& cell = (*this)(r, c);
    if (cell.isFlagged()) { return false; }
    if (!cell.reveal(&revealCount)) { return false; }
    if (cell.isMine()) { return true; }
    if (cell.getMineCount() != 0) { return false; }

    // reveal neighbors
    for (int i = 0; i <= 8; i++) {
      if (i == 4) { continue; } // skip self
      reveal(r + i / 3 - 1, c + i % 3 - 1);
    }

    return false;
  }
  void revealAllMines() {
    for (size_t i = 0; i < nRows * nCols; i++) {
      if (grid[i].isMine()) { grid[i].reveal(&revealCount); }
    }
  }
  int countFlaggedNeighbors(size_t r, size_t c) {
    int count = 0;
    for (int i = 0; i <= 7; i++) {
      Cell* neighbor = getNthNeighbor(i, r, c);
      if (neighbor == nullptr) { continue; }
      if (neighbor->isFlagged()) { count++; }
    }
    return count;
  }
};
