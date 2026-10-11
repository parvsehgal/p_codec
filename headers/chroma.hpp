#pragma once
#include <string>
#include <vector>
using namespace std;

class chroma {
public:
  unsigned int height;
  unsigned int width;

  vector<unsigned char> readFile(unsigned int height, unsigned int width,
                                 std::string filePath);

  vector<unsigned char> rgbToyuv(std::vector<unsigned char> &rgbBuffer,
                                 unsigned int height, unsigned int width);

  vector<unsigned char> generateSubsample(unsigned int height,
                                          unsigned int width,
                                          std::string filePath);
  //=========DECODE LOGIC=======
  vector<unsigned char> yuvTorgb(const vector<unsigned char> &decodedSubSample,
                                 int width, int height);
};
