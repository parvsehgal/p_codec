#include "headers/coder.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main() {
  string mode;
  cout << "encode or decode?" << endl;
  cin >> mode;

  coder coderObj;

  if (mode == "decode") {
    unsigned int height;
    unsigned int width;
    string compressedFilePath;
    string outputFile;

    cout << "enter ORIGINAL Width and Height (same values used to encode)"
         << endl;
    cin >> width;
    cin >> height;
    cout << "enter path to compressed file" << endl;
    cin >> compressedFilePath;
    cout << "enter name for the decoded raw RGB output file" << endl;
    cin >> outputFile;

    coderObj.decode(height, width, compressedFilePath, outputFile);
    return 0;
  }

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

  vector<unsigned char> compresedFile =
      coderObj.encode(height, width, filePath);
  ofstream resultantFile{outputFile, std::ios::binary};
  resultantFile.write(reinterpret_cast<char *>(compresedFile.data()),
                      compresedFile.size());

  return 0;
}
