#include "headers/chroma.hpp"
#include <cstddef>
#include <iostream>
#include <string>
using namespace std;

int main() {
  // get all input
  unsigned int height;
  unsigned int width;
  string filePath;
  cout << "enter Height and width" << endl;
  cin >> height;
  cin >> width;
  cout << "enter filePath" << endl;
  cin >> filePath;
  chroma chromaObj;
  vector<unsigned char> imageSubSample =
      chromaObj.generateSubsample(height, width, filePath);
}
