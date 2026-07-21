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
    return buffer;
  }
  vector<unsigned char> rgbToyuv(vector<unsigned char> &rgbBuffer,
                                 unsigned int height, unsigned int width) {
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
      if (currCb < 0 || currCr < 0) {
      }
      yBuffer.push_back(currY);
      cbBuffer.push_back(currCb);
      crBuffer.push_back(currCr);
    }
    vector<unsigned char> yuvBuffer;
    yuvBuffer.reserve(yBuffer.size() + cbBuffer.size() + crBuffer.size());
    yuvBuffer.insert(yuvBuffer.end(), yBuffer.begin(), yBuffer.end());
    yuvBuffer.insert(yuvBuffer.end(), cbBuffer.begin(), cbBuffer.end());
    yuvBuffer.insert(yuvBuffer.end(), crBuffer.begin(), crBuffer.end());

    // subsample here only 1.(noisy subsample)
    vector<unsigned char> reducedCb;
    vector<unsigned char> reducedCr;
    for (int i = 0; i < height; i += 2) {
      for (int j = 0; j < width; j += 2) {
        reducedCb.push_back(cbBuffer[(i * width) + j]);
        reducedCr.push_back(crBuffer[(i * width) + j]);
      }
    }
    vector<unsigned char> yuv420pbuffer;
    yuv420pbuffer.reserve(yBuffer.size() + reducedCb.size() + reducedCr.size());
    yuv420pbuffer.insert(yuv420pbuffer.end(), yBuffer.begin(), yBuffer.end());
    yuv420pbuffer.insert(yuv420pbuffer.end(), reducedCb.begin(),
                         reducedCb.end());
    yuv420pbuffer.insert(yuv420pbuffer.end(), reducedCr.begin(),
                         reducedCr.end());
    cout << "444P size ->" << yuvBuffer.size() << endl;
    cout << "420P size ->" << yuv420pbuffer.size() << " " << endl;
    return yuv420pbuffer;
  }

  void generateSubsample(unsigned int height, unsigned int width,
                         string filePath) {
    vector<unsigned char> buffer = readFile(height, width, filePath);
    vector<unsigned char> yuvBuffer = rgbToyuv(buffer, height, width);
    ofstream outputFile{"customPaul.raw", ios::binary};
    outputFile.write(reinterpret_cast<char *>(yuvBuffer.data()),
                     yuvBuffer.size());
    cout << "customPaul size " << outputFile.tellp() << " ";
  }
};

int main() {
  chroma chromaObj;
  string filePath = "./raw_images/aimer.raw";
  chromaObj.generateSubsample(1594, 736, filePath);
}
