#pragma once
#include <string>
#include <vector>

class chroma {
public:
  std::vector<unsigned char> readFile(unsigned int height, unsigned int width,
                                      std::string filePath);

  std::vector<unsigned char> rgbToyuv(std::vector<unsigned char> &rgbBuffer,
                                      unsigned int height, unsigned int width);

  void generateSubsample(unsigned int height, unsigned int width,
                         std::string filePath);
};
