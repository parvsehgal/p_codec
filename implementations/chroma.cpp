#include "../headers/chroma.hpp"
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

vector<unsigned char> chroma ::readFile(unsigned int height, unsigned int width,
                                        string filePath) {
  size_t fileSize = height * width * 3;
  ifstream inputFile{filePath};
  vector<unsigned char> rgbBuffer(fileSize);
  inputFile.read(reinterpret_cast<char *>(rgbBuffer.data()), fileSize);

  unsigned int heightAdj = (16 - height % 16) % 16;
  unsigned int widthAdj = (16 - width % 16) % 16;
  vector<vector<unsigned char>> rgbMaxtix(
      (height + heightAdj), vector<unsigned char>((width + widthAdj) * 3));

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < 3 * (width); j++) {
      rgbMaxtix[i][j] = rgbBuffer[i * (3 * width) + j];
    }
  }

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
  vector<unsigned char> rgbBuffer =
      readFile(this->height, this->width, filePath);
  unsigned int heightAdj = (16 - height % 16) % 16;
  unsigned int widthAdj = (16 - width % 16) % 16;
  this->height += heightAdj;
  this->width += widthAdj;
  vector<unsigned char> yuv420pbuffer =
      rgbToyuv(rgbBuffer, this->height, this->width);
  return yuv420pbuffer;
}
//==============DECODE LOGIC=================
// this function is AI generated
vector<unsigned char> chroma::yuvTorgb(const vector<unsigned char> &yuv,
                                       int width, int height) {
  const size_t lumaSize = static_cast<size_t>(width) * height;
  const int cw = width / 2; // chroma plane width
  const size_t chromaSize = static_cast<size_t>(cw) * (height / 2);

  vector<unsigned char> rgb;
  if (yuv.size() < lumaSize + 2 * chromaSize) {
    cerr << "yuvTorgb: buffer too small" << endl;
    return rgb;
  }
  rgb.resize(lumaSize * 3);

  const unsigned char *Y = yuv.data();
  const unsigned char *Cb = Y + lumaSize;
  const unsigned char *Cr = Cb + chromaSize;

  auto clamp = [](float v) -> unsigned char {
    int p = static_cast<int>(lround(v));
    return static_cast<unsigned char>(min(255, max(0, p)));
  };

  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      // each chroma sample covers a 2x2 block of luma pixels
      size_t ci = static_cast<size_t>(row / 2) * cw + (col / 2);

      float y = Y[static_cast<size_t>(row) * width + col];
      float cb = Cb[ci] - 128.0f;
      float cr = Cr[ci] - 128.0f;

      size_t o = (static_cast<size_t>(row) * width + col) * 3;
      rgb[o + 0] = clamp(y + 1.402f * cr);                     // R
      rgb[o + 1] = clamp(y - 0.344136f * cb - 0.714136f * cr); // G
      rgb[o + 2] = clamp(y + 1.772f * cb);                     // B
    }
  }
  return rgb;
}
