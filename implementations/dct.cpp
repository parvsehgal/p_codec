#include "../headers/dct.hpp"
#include <cstddef>
#include <iostream>
using namespace std;

void dct::dctOn8x8(int i, int j, vector<unsigned char> &buffer) {}
void dct::performDCT(vector<unsigned char> &imageSubSample, unsigned int width,
                     unsigned int height) {
  this->height = height;
  this->width = width;
  cout << "control flow reached here----------------------------------" << endl;

  // split the 3 componensts into their own vectors
  // make 8x8 blocks and perform DCT on these blocks
  size_t totalBufferSize = imageSubSample.size();
  cout << "totalBufferSize" << totalBufferSize << endl;
  int lumaSize = (totalBufferSize * 2) / 3;
  vector<unsigned char> lumaBuffer = {imageSubSample.begin(),
                                      imageSubSample.begin() + lumaSize};
  vector<unsigned char> chromaBuffer = {imageSubSample.begin() + lumaSize,
                                        imageSubSample.end()};
  cout << "lumaSize" << lumaBuffer.size() << " " << lumaSize << endl;
  cout << "chromaSize" << chromaBuffer.size() << " "
       << totalBufferSize - lumaSize << endl;

  // now go over the lumaBuffer and send 8x8 blcoks to DCT function
  for (int i = 0; i < height; i += 7) {
    for (int j = 0; j < width; j += 7) {
      cout << i << " " << j << endl;
      dctOn8x8(i, j, lumaBuffer);
    }
  }
}
