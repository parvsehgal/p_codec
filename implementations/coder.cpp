#include "../headers/coder.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
using namespace std;

vector<unsigned char> coder::encode(unsigned int height, unsigned int width,
                                    string file_path) {
  vector<unsigned char> imageSubSample =
      this->chromaObj.generateSubsample(height, width, file_path);
  ofstream outputFile{"coded.yuv"};
  outputFile.write(reinterpret_cast<char *>(imageSubSample.data()),
                   imageSubSample.size());

  unsigned int heightAdj = (16 - height % 16) % 16;
  unsigned int widthAdj = (16 - width % 16) % 16;
  height += heightAdj;
  width += widthAdj;

  auto [yMatrix, cbMatrix, crMatrix] =
      this->dctObj.performDCT(imageSubSample, width, height);
  vector<unsigned char> compressedFile =
      this->entropyObj.runLevel({yMatrix, cbMatrix, crMatrix}, width, height);
  cout << "UNCOMPRESSED FILE SIZE= " << imageSubSample.size() * 2 << endl;
  cout << "COMPRESSED FILE SIZE= " << compressedFile.size() << endl;
  return compressedFile;
}
//=================DECODE LOGIC================
void coder ::decode(string compressedFileName) {
  // extract the height and width from the compressedFile
  ifstream compressedFile{compressedFileName};
  vector<unsigned char> dims(4);
  compressedFile.read(reinterpret_cast<char *>(dims.data()), dims.size());
  int width = static_cast<int>((dims[0] << 8) | (dims[1]));
  int height = static_cast<int>((dims[2] << 8) | (dims[3]));

  auto decodedBlocks =
      this->entropyObj.reverseEntropy(width, height, compressedFileName);
  vector<unsigned char> decodedSubSample =
      this->dctObj.performIDCT(width, height, decodedBlocks);
  ofstream out("decoded.yuv", ios::binary);
  out.write(reinterpret_cast<const char *>(decodedSubSample.data()),
            decodedSubSample.size());
  out.close();

  auto rgbBuffer = this->chromaObj.yuvTorgb(decodedSubSample, width, height);
  ofstream outRGB("decoded.rgb", ios::binary);
  outRGB.write(reinterpret_cast<char *>(rgbBuffer.data()), rgbBuffer.size());
  outRGB.close();
}
