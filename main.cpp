#include "headers/coder.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main() {
  unsigned int height;
  unsigned int width;
  string filePath;
  string outputFile;
  cout << "enter Width and Height" << endl;
  cin >> width;
  cin >> height;
  cout << "enter filePath" << endl;
  cin >> filePath;
  cout << "enter name for the compressed file";
  cin >> outputFile;

  coder coderObj;
  vector<unsigned char> compresedFile =
      coderObj.encode(height, width, filePath);
  ofstream resultantFile{outputFile, std::ios::binary};
  resultantFile.write(reinterpret_cast<char *>(compresedFile.data()),
                      compresedFile.size());
  // start the decode pipeline would be the inverse of the decode process
  //
}
