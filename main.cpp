#include "headers/chroma.hpp"
#include "headers/dct.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main() {
  // get all input
  unsigned int height;
  unsigned int width;
  string filePath;
  cout << "enter Width and Height" << endl;
  cin >> width;
  cin >> height;
  cout << "enter filePath" << endl;
  cin >> filePath;
  chroma chromaObj;
  vector<unsigned char> imageSubSample =
      chromaObj.generateSubsample(height, width, filePath);
  ofstream outputFile{"subSample.raw"};
  outputFile.write(reinterpret_cast<char *>(imageSubSample.data()),
                   imageSubSample.size());
  unsigned int heightAdj = (16 - height % 16) % 16;
  unsigned int widthAdj = (16 - width % 16) % 16;
  height += heightAdj;
  width += widthAdj;
  dct dctObj;
  dctObj.performDCT(imageSubSample, width, height);
}
