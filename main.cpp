#include <cstddef>
#include <fstream>
#include <iostream>
using namespace std;

void read_file(string file_path) {
  ifstream file{file_path, ios::ate | ios::binary};
  cout << file.tellg() << endl;
}
int main() {
  string currPath = "raw_images/paul.raw";
  read_file(currPath);
}
