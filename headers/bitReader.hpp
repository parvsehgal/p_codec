#pragma once
#include <cstdint>
#include <vector>
using namespace std;

// Mirror of bitWriter, but for consuming a bitstream MSB-first.
// Because bitWriter::flush() only happens once at the very end of encoding,
// this reader maintains a persistent bit cursor across the whole buffer --
// there is no per-block byte realignment.
class bitReader {
public:
  bitReader(const vector<unsigned char> &data);

  // Reads a single bit. Returns 0 if past the end of the buffer (so a
  // caller mid-VLC-match near EOF degrades gracefully instead of crashing).
  int readBit();

  // Reads n bits (MSB first) and returns them right-aligned in a uint32_t.
  uint32_t readBits(int n);

  bool hasMoreBits() const;

private:
  const vector<unsigned char> &buffer;
  size_t byteIndex = 0;
  int bitIndex = 0; // next bit to read within buffer[byteIndex], 0 = MSB
};
