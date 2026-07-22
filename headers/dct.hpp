#pragma once
#include <vector>
using namespace std;

class dct {
public:
  unsigned int width;
  unsigned int height;

  void performDCT(vector<unsigned char> &imageSubSample, unsigned int width,
                  unsigned int height);
  void dctOn8x8(int i, int j, vector<unsigned char> &buffer);
};
