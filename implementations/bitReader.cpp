#include "../headers/bitReader.hpp"
#include <stdexcept>
#include <vector>
using namespace std;

void bitReader::init(vector<unsigned char> &encodedFile) {
  this->buffer = encodedFile;
  this->currentByte = 0;
  this->currentPos = 0;
  this->bufferSize = encodedFile.size();
}

int bitReader::getBit() {
  // what is there is no more space to read the file anymore
  if (this->currentByte >= this->bufferSize) {
    throw runtime_error("cant read the buffer anymore");
  }
  // get currentByte and return thr currentBit
  int byte = this->buffer[currentByte];
  int bit = (byte >> (8 - this->currentPos - 1)) & 1;
  // update poisitions before sending the result
  this->currentPos++;
  if (this->currentPos >= 8) {
    this->currentPos = 0;
    this->currentByte++;
  }
  return bit;
}

bool bitReader::canRead() { return !(this->currentByte >= this->bufferSize); }
