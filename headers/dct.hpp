#pragma once
#include <vector>
using namespace std;

class dct {
public:
  unsigned int width;
  unsigned int height;

  void performDCT(vector<unsigned char> &imageSubSample);
};
