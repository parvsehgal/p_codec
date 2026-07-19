#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;

class chroma {
public:
  vector<unsigned char> readFile(unsigned int height, unsigned int width,
                                 string filePath) {
    ifstream file{filePath, ios::binary};
    size_t bufferSize = height * width * 3;
    vector<unsigned char> buffer(bufferSize);
    file.read(reinterpret_cast<char *>(buffer.data()), bufferSize);
    cout << file.tellg() << endl;
    return buffer;
  }
  void generateSubsample(unsigned int height, unsigned int width,
                         string filePath) {
    vector<unsigned char> buffer = readFile(height, width, filePath);
    cout << (int)buffer[0] << " " << (int)buffer[1] << " " << (int)buffer[2];
  }
};

int main() {
  chroma chromaObj;
  string filePath = "./raw_images/paul.raw";
  chromaObj.generateSubsample(736, 1124, filePath);
}
