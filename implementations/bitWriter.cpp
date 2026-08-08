#include "../headers/bitWriter.hpp"
#include <cstdint>
#include <vector>

void bitWriter::addBits(uint32_t bits, int bitLength) {
  for (int i = bitLength - 1; i >= 0; i--) {
    int bit = (bits >> i) & 1;
    this->currByte = (this->currByte << 1) | bit;
    ++this->bitCount;
    if (this->bitCount == 8) {
      this->buffer.push_back(this->currByte);
      this->bitCount = 0;
      this->currByte = 0;
    }
  }
}

void bitWriter::flush() {
  if (this->bitCount > 0) {
    this->currByte = currByte << (8 - this->bitCount);
    buffer.push_back(this->currByte);
    this->currByte = 0;
    this->bitCount = 0;
  }
}
