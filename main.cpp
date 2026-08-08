#include "headers/coder.hpp"
#include <cstddef>
#include <iostream>
#include <string>
using namespace std;

int main() {
  unsigned int height;
  unsigned int width;
  string filePath;
  cout << "enter Width and Height" << endl;
  cin >> width;
  cin >> height;
  cout << "enter filePath" << endl;
  cin >> filePath;

  coder coderObj;
  coderObj.encode(height, width, filePath);
}
