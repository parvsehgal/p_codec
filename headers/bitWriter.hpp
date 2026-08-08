#pragma once
#include <cstdint>
#include <vector>
using namespace std;

class bitWriter {
public:
  void addBits(uint32_t, int);
  void flush();

  vector<unsigned char> buffer;
  unsigned char currByte = 0;
  int bitCount = 0;
};
