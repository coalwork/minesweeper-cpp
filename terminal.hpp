#pragma once

#include <unistd.h>
#include <termios.h>
#include <cstdio>
#include <iostream>

// code from https://stackoverflow.com/a/6698188

// Initialize new terminal i/o settings
static struct termios old, new1;
void initTermios(int echo) {
  tcgetattr(0, &old);                  // grab old terminal i/o settings
  new1 = old;                          // make new settings same as old settings
  new1.c_lflag &= ~ICANON;             // disable buffered i/o
  new1.c_lflag &= echo ? ECHO : ~ECHO; // set echo mode
  tcsetattr(0, TCSANOW, &new1);        // use these new terminal i/o settings now
}

// Restore old terminal i/o settings
void resetTermios() {
  tcsetattr(0, TCSANOW, &old);
}

std::string getCursorPosition() {
  size_t bufSize = 10;

  std::cout << "\x1b[6n";

  char positionInfo[bufSize] = {0};

  for (int i = 0; i < bufSize; i++) {
    std::cin.read(positionInfo + i, 1);

    if (positionInfo[i] == 'R') { break; }
  }

  return std::string(positionInfo);
}

void parsePosition(std::string position, int &row, int &column) {
  position.erase(0, 2);
  position.erase(position.end() - 1);

  size_t delimiterPosition = position.find(';');
  row = std::stoi(position.substr(0, delimiterPosition));
  column = stoi(std::string(position.c_str() + delimiterPosition + 1));
}
