#include "../headers/bitReader.hpp"

bitReader::bitReader(const vector<unsigned char> &data) : buffer(data) {}

int bitReader::readBit() {
  if (byteIndex >= buffer.size()) {
    return 0;
  }
  unsigned char byte = buffer[byteIndex];
  int bit = (byte >> (7 - bitIndex)) & 1;
  bitIndex++;
  if (bitIndex == 8) {
    bitIndex = 0;
    byteIndex++;
  }
  return bit;
}

uint32_t bitReader::readBits(int n) {
  uint32_t result = 0;
  for (int i = 0; i < n; i++) {
    result = (result << 1) | static_cast<uint32_t>(readBit());
  }
  return result;
}

bool bitReader::hasMoreBits() const { return byteIndex < buffer.size(); }
