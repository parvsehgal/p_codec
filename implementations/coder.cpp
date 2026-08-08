#include "../headers/coder.hpp"
#include <fstream>
#include <iostream>
using namespace std;

vector<unsigned char> coder::encode(unsigned int height, unsigned int width,
                                    string file_path) {
  vector<unsigned char> imageSubSample =
      this->chromaObj.generateSubsample(height, width, file_path);
  ofstream outputFile{"codec.raw"};
  outputFile.write(reinterpret_cast<char *>(imageSubSample.data()),
                   imageSubSample.size());

  unsigned int heightAdj = (16 - height % 16) % 16;
  unsigned int widthAdj = (16 - width % 16) % 16;
  height += heightAdj;
  width += widthAdj;

  auto [yMatrix, cbMatrix, crMatrix] =
      this->dctObj.performDCT(imageSubSample, width, height);
  vector<unsigned char> compressedFile =
      this->entropyObj.runLevel({yMatrix, cbMatrix, crMatrix});
  cout << "UNCOMPRESSED FILE SIZE= " << imageSubSample.size() << endl;
  cout << "COMPRESSED FILE SIZE= " << compressedFile.size() << endl;
  return compressedFile;
}
