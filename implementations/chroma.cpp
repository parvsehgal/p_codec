#include "../headers/chroma.hpp"
#include <algorithm>
#include <cmath>
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
  cout << "raw image size ->" << inputFile.tellg() << endl;
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
  // make the final 420p buffer and return it
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
  cout << "new Dimensions " << this->width << " " << this->height << endl;
  return yuv420pbuffer;
}

// ============================= DECODE SIDE =============================

vector<unsigned char> chroma::upsampleChroma(vector<unsigned char> &chromaBuffer,
                                             unsigned int fullHeight,
                                             unsigned int fullWidth) {
  // chromaBuffer is (fullHeight/2) x (fullWidth/2). Each sample was the
  // average of a 2x2 luma-resolution block on encode, so nearest-neighbor
  // replicate it back across that 2x2 region.
  unsigned int halfWidth = fullWidth / 2;
  vector<unsigned char> upsampled(fullHeight * fullWidth);
  for (unsigned int i = 0; i < fullHeight; i += 2) {
    for (unsigned int j = 0; j < fullWidth; j += 2) {
      unsigned char sample = chromaBuffer[(i / 2) * halfWidth + (j / 2)];
      upsampled[i * fullWidth + j] = sample;
      upsampled[i * fullWidth + j + 1] = sample;
      upsampled[(i + 1) * fullWidth + j] = sample;
      upsampled[(i + 1) * fullWidth + j + 1] = sample;
    }
  }
  return upsampled;
}

vector<unsigned char> chroma::yuvToRgb(vector<unsigned char> &yBuffer,
                                       vector<unsigned char> &cbBuffer,
                                       vector<unsigned char> &crBuffer,
                                       unsigned int height,
                                       unsigned int width) {
  // Invert:
  //   Y  = 0.299 R + 0.587 G + 0.114 B
  //   Cb = 128 + 0.564 (B - Y)
  //   Cr = 128 + 0.713 (R - Y)
  vector<unsigned char> rgbBuffer;
  rgbBuffer.reserve(static_cast<size_t>(height) * width * 3);
  for (unsigned int idx = 0; idx < height * width; idx++) {
    double Y = static_cast<double>(yBuffer[idx]);
    double Cb = static_cast<double>(cbBuffer[idx]);
    double Cr = static_cast<double>(crBuffer[idx]);

    double B = Y + (Cb - 128.0) / 0.564;
    double R = Y + (Cr - 128.0) / 0.713;
    double G = (Y - 0.299 * R - 0.114 * B) / 0.587;

    auto clamp255 = [](double v) {
      int iv = static_cast<int>(round(v));
      return static_cast<unsigned char>(max(0, min(255, iv)));
    };

    rgbBuffer.push_back(clamp255(R));
    rgbBuffer.push_back(clamp255(G));
    rgbBuffer.push_back(clamp255(B));
  }
  return rgbBuffer;
}

void chroma::writeFile(vector<unsigned char> &rgbBuffer,
                       unsigned int paddedHeight, unsigned int paddedWidth,
                       unsigned int origHeight, unsigned int origWidth,
                       string outputPath) {
  vector<unsigned char> cropped;
  cropped.reserve(static_cast<size_t>(origHeight) * origWidth * 3);
  for (unsigned int i = 0; i < origHeight; i++) {
    for (unsigned int j = 0; j < origWidth * 3; j++) {
      cropped.push_back(rgbBuffer[i * (paddedWidth * 3) + j]);
    }
  }
  ofstream outputFile{outputPath, std::ios::binary};
  outputFile.write(reinterpret_cast<char *>(cropped.data()), cropped.size());
}

void chroma::decode(vector<unsigned char> &yuvBuffer, unsigned int paddedHeight,
                    unsigned int paddedWidth, unsigned int origHeight,
                    unsigned int origWidth, string outputPath) {
  size_t lumaSize = static_cast<size_t>(paddedHeight) * paddedWidth;
  size_t chromaSize = lumaSize / 4;

  vector<unsigned char> yBuffer(yuvBuffer.begin(), yuvBuffer.begin() + lumaSize);
  vector<unsigned char> cbHalf(yuvBuffer.begin() + lumaSize,
                               yuvBuffer.begin() + lumaSize + chromaSize);
  vector<unsigned char> crHalf(yuvBuffer.begin() + lumaSize + chromaSize,
                               yuvBuffer.begin() + lumaSize + 2 * chromaSize);

  vector<unsigned char> cbFull = upsampleChroma(cbHalf, paddedHeight, paddedWidth);
  vector<unsigned char> crFull = upsampleChroma(crHalf, paddedHeight, paddedWidth);

  vector<unsigned char> rgbBuffer =
      yuvToRgb(yBuffer, cbFull, crFull, paddedHeight, paddedWidth);

  writeFile(rgbBuffer, paddedHeight, paddedWidth, origHeight, origWidth,
           outputPath);
}
