#include "../headers/bitReader.hpp"
#include <iostream>
#include <vector>
using namespace std;

void bitReader::reader(vector<unsigned char> data) {
  this->buffer = data;
  // start reading
  for (int i = 0; i < data.size(); i++) {
    this->bytePos = i;
    unsigned char currByte = data[i];
    for (int j = 7; j >= 0; j--) {
      this->currBit = (currByte >> j) & 1;
      cout << this->currBit;
      this->currBit = 0;
    }
  }
}
