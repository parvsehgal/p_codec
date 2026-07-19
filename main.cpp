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
    cout << "file.tellg()-> " << file.tellg() << endl;
    return buffer;
  }
  vector<unsigned char> rgbToyuv(vector<unsigned char> &rgbBuffer) {
    // okay so have 3 sep vectors to store Y CR and CBs for each pixel and then
    // just concatinate them
    vector<unsigned char> yBuffer;
    vector<unsigned char> cbBuffer;
    vector<unsigned char> crBuffer;
    for (int i = 0; i < rgbBuffer.size(); i += 3) {
      unsigned char currY = static_cast<unsigned int>(0.299 * rgbBuffer[i] +
                                                      0.587 * rgbBuffer[i + 1] +
                                                      0.114 * rgbBuffer[i + 2]);
      unsigned char currCb =
          static_cast<unsigned int>(128 + 0.564 * (rgbBuffer[i + 2] - currY));
      unsigned char currCr =
          static_cast<unsigned int>(128 + 0.713 * (rgbBuffer[i] - currY));
      yBuffer.push_back(currY);
      cbBuffer.push_back(currCb);
      crBuffer.push_back(currCr);
    }
    vector<unsigned char> yuvBuffer;
    yuvBuffer.reserve(yBuffer.size() + cbBuffer.size() + crBuffer.size());
    yuvBuffer.insert(yuvBuffer.end(), yBuffer.begin(), yBuffer.end());
    yuvBuffer.insert(yuvBuffer.end(), cbBuffer.begin(), cbBuffer.end());
    yuvBuffer.insert(yuvBuffer.end(), crBuffer.begin(), crBuffer.end());
    return yuvBuffer;
  }
  void generateSubsample(unsigned int height, unsigned int width,
                         string filePath) {
    vector<unsigned char> buffer = readFile(height, width, filePath);
    vector<unsigned char> yuvBuffer = rgbToyuv(buffer);
    cout << "yuvBuffer.size()-> " << yuvBuffer.size() << endl;
    ofstream outputFile{"customPaul.raw"};
    outputFile.write(reinterpret_cast<char *>(yuvBuffer.data()),
                     yuvBuffer.size());
  }
};

int main() {
  chroma chromaObj;
  string filePath = "./raw_images/paul.raw";
  chromaObj.generateSubsample(736, 1124, filePath);
}
