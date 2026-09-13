#include "headers/coder.hpp"
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

int main() {
  int choice;
  cout << "enter 1 to encode and 0 to decode " << endl;
  cin >> choice;
  if (choice) {
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
  } else {
    // start the decode pipeline would be the inverse of the decode process
    cout << "enter the compressed file to be decoded" << endl;
    string compressedFileName;
    cin >> compressedFileName;
    coder coderObj;
    coderObj.decode(compressedFileName);
  }
}
