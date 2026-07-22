#include "../headers/chroma.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

vector<unsigned char> chroma ::readFile(unsigned int height, unsigned int width,
                                        string filePath) {
  // read the RGB values of the file
  size_t fileSize = height * width * 3;
  ifstream inputFile{filePath};
  vector<unsigned char> rgbBuffer(fileSize);
  inputFile.read(reinterpret_cast<char *>(rgbBuffer.data()), fileSize);
  cout << "raw paul size ->" << inputFile.tellg() << endl;

  // now make padded rgbMatrix;
  unsigned int heightAdj = (16 - height % 16) % 16;
  unsigned int widthAdj = (16 - width % 16) % 16;
  vector<vector<unsigned char>> rgbMaxtix(
      (height + heightAdj), vector<unsigned char>((width + widthAdj) * 3));

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < 3 * (width); j++) {
      rgbMaxtix[i][j] = rgbBuffer[i * (3 * width) + j];
    }
  }

  // now fill the unfilled cells of the matrix to pad it
  if (heightAdj) {
    for (unsigned int i = height; i < (height + heightAdj); i++) {
      for (unsigned int j = 0; j < 3 * (width); j++) {
        rgbMaxtix[i][j] = rgbMaxtix[i - 1][j];
      }
    }
  }
  if (widthAdj) {
    for (unsigned int i = 0; i < (height + heightAdj); i++) {
      for (unsigned int j = 3 * width; j < (3 * (width + widthAdj)); j++) {
        rgbMaxtix[i][j] = rgbMaxtix[i][j - 3];
      }
    }
  }

  // now convert padded rgbMatrix to 1D vector and rerturn it
  vector<unsigned char> paddedRGBbuffer;
  paddedRGBbuffer.reserve((height + heightAdj) * (width + widthAdj) * 3);
  for (int i = 0; i < (height + heightAdj); i++) {
    for (int j = 0; j < 3 * (width + widthAdj); j++) {
      paddedRGBbuffer.push_back(rgbMaxtix[i][j]);
    }
  }
  return paddedRGBbuffer;
}
vector<unsigned char> chroma::rgbToyuv(vector<unsigned char> &rgbBuffer,
                                       unsigned int height,
                                       unsigned int width) {
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
  // subsample here take avg of every 4 samples (no index out of bounds due to
  // padding);
  vector<unsigned char> reducedCb;
  vector<unsigned char> reducedCr;
  for (int i = 0; i < height; i += 2) {
    for (int j = 0; j < width; j += 2) {
      unsigned int avgCb =
          (cbBuffer[(i * width) + j] + cbBuffer[(i * width) + j + 1] +
           cbBuffer[((i + 1) * width) + j] +
           cbBuffer[((i + 1) * width) + j + 1]) /
          4;
      unsigned int avgCr =
          (crBuffer[(i * width) + j] + crBuffer[(i * width) + j + 1] +
           crBuffer[((i + 1) * width) + j] +
           crBuffer[((i + 1) * width) + j + 1]) /
          4;

      reducedCb.push_back(avgCb);
      reducedCr.push_back(avgCr);
    }
  }
  vector<unsigned char> yuv420pbuffer;
  yuv420pbuffer.reserve(yBuffer.size() + reducedCb.size() + reducedCr.size());
  yuv420pbuffer.insert(yuv420pbuffer.end(), yBuffer.begin(), yBuffer.end());
  yuv420pbuffer.insert(yuv420pbuffer.end(), reducedCb.begin(), reducedCb.end());
  yuv420pbuffer.insert(yuv420pbuffer.end(), reducedCr.begin(), reducedCr.end());
  return yuv420pbuffer;
}

vector<unsigned char> chroma::generateSubsample(unsigned int height,
                                                unsigned int width,
                                                string filePath) {
  this->height = height;
  this->width = width;
  vector<unsigned char> rgbBuffer = readFile(height, width, filePath);
  unsigned int heightAdj = (16 - height % 16) % 16;
  unsigned int widthAdj = (16 - width % 16) % 16;
  this->height += heightAdj;
  this->width += widthAdj;
  vector<unsigned char> yuv420pbuffer =
      rgbToyuv(rgbBuffer, this->height, this->width);
  return yuv420pbuffer;
}
