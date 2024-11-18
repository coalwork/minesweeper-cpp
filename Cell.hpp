#pragma once

class Cell {
  bool revealed = false;
  bool flagged = false;
  int mineCount = 0; // adjacent mine count, -1 if mine

  public:
  bool isRevealed() const { return revealed; }
  bool reveal(int *revealCount) {
    if (revealed) { return false; }
    revealed = true;
    (*revealCount)++;
    return true;
  }
  bool isFlagged() const { return flagged; }
  bool toggleFlag() {
    if (revealed) { return false; }
    flagged = !flagged;
    return true;
  }
  void incrementMineCount() { if (!isMine()) { mineCount++; } }
  void setMine() { mineCount = -1; }
  bool isMine() const { return mineCount == -1; }
  int getMineCount() const { return mineCount; }
};
