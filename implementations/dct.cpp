#include "../headers/dct.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <tuple>
#include <vector>
#define PI 3.1415926535897
using namespace std;

void dct::dctOn8x8(unsigned int i, unsigned int j,
                   vector<vector<float>> &matrix, string component) {
  float Cu, Cv;
  vector<vector<int>> input(8, vector<int>(8));
  for (int a = 0; a < 8; a++) {
    for (int b = 0; b < 8; b++) {
      input[a][b] = static_cast<int>(matrix[i + a][j + b]);
    }
  }
  // now should be able to perform DCT and put the values directly in the source
  // matrix
  for (int u = 0; u < 8; u++) {
    for (int v = 0; v < 8; v++) {
      double sum = 0.0;
      for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
          sum = sum + input[x][y] * cos(((2.0 * x + 1) * u * PI) / 16.0) *
                          cos(((2.0 * y + 1) * v * PI) / 16.0);
        }
      }
      if (u == 0)
        Cu = 1 / sqrt(2);
      else
        Cu = 1;
      if (v == 0)
        Cv = 1 / sqrt(2);
      else
        Cv = 1;
      if (component == "lume") {
        matrix[u + i][v + j] =
            round(1 / 4.0 * Cu * Cv * sum / this->luminanceTable[u][v]);
      } else {
        matrix[u + i][v + j] =
            round(1 / 4.0 * Cu * Cv * sum / this->chrominanceTabe[u][v]);
      }
    }
  }
  if (i == 0 && j == 0) {
    cout << "Before DCT/Quant" << endl;
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        cout << input[row][col] << " ";
      }
      cout << endl;
    }
    cout << "after DCT/Quant" << endl;
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        cout << matrix[row + i][col + j] << " ";
      }
      cout << endl;
    }
  }
}

tuple<vector<vector<float>>, vector<vector<float>>, vector<vector<float>>>
dct::performDCT(vector<unsigned char> &imageSubSample, unsigned int width,
                unsigned int height) {
  this->width = width;
  this->height = height;
  size_t totalSize = imageSubSample.size();
  unsigned int lumaSize = (totalSize * 2) / 3;
  vector<unsigned char> yBuffer = {imageSubSample.begin(),
                                   imageSubSample.begin() + lumaSize};
  unsigned int chromaSize = (totalSize - lumaSize) / 2;
  vector<unsigned char> cbBuffer = {imageSubSample.begin() + lumaSize,
                                    imageSubSample.begin() + lumaSize +
                                        chromaSize};
  vector<unsigned char> crBuffer = {
      imageSubSample.begin() + lumaSize + chromaSize, imageSubSample.end()};
  cout << "ybuffer size -> " << yBuffer.size() << endl;
  cout << "cbBuffer size -> " << cbBuffer.size() << endl;
  cout << "crBuffer size -> " << crBuffer.size() << endl;
  // now I have 3 sperate buffer which should be correctly split
  // performDCT on 8x8 samples of these buffers after making them into matrices
  vector<vector<float>> yMatrix(height, vector<float>(width));
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      yMatrix[i][j] = static_cast<float>(yBuffer[i * width + j] - 128);
    }
  }
  vector<vector<float>> cbMatrix(height / 2, vector<float>(width / 2));
  vector<vector<float>> crMatrix(height / 2, vector<float>(width / 2));
  for (int i = 0; i < (height / 2); i++) {
    for (int j = 0; j < (width / 2); j++) {
      cbMatrix[i][j] = static_cast<float>(cbBuffer[i * (width / 2) + j] - 128);
      crMatrix[i][j] = static_cast<float>(crBuffer[i * (width / 2) + j] - 128);
    }
  }
  // all matrices made now perform DCT on 8x8 blocks
  for (int i = 0; i < yMatrix.size(); i += 8) {
    for (int j = 0; j < yMatrix[0].size(); j += 8) {
      dctOn8x8(i, j, yMatrix, "lume");
    }
  }
  for (int i = 0; i < cbMatrix.size(); i += 8) {
    for (int j = 0; j < cbMatrix[0].size(); j += 8) {
      dctOn8x8(i, j, cbMatrix, "chrome");
      dctOn8x8(i, j, crMatrix, "chrome");
    }
  }
  // now all matrices should have DCT applied on them now return to main
  return {yMatrix, cbMatrix, crMatrix};
}

// ============================= DECODE SIDE =============================

void dct::idctOn8x8(unsigned int i, unsigned int j, vector<vector<float>> &matrix,
                    string component) {
  // matrix[i+u][j+v] currently holds the quantized DCT coefficient F(u,v).
  vector<vector<double>> coeff(8, vector<double>(8));
  for (int u = 0; u < 8; u++) {
    for (int v = 0; v < 8; v++) {
      int quant = (component == "lume") ? this->luminanceTable[u][v]
                                        : this->chrominanceTabe[u][v];
      coeff[u][v] = static_cast<double>(matrix[i + u][j + v]) * quant;
    }
  }

  double Cu, Cv;
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      double sum = 0.0;
      for (int u = 0; u < 8; u++) {
        for (int v = 0; v < 8; v++) {
          Cu = (u == 0) ? 1.0 / sqrt(2.0) : 1.0;
          Cv = (v == 0) ? 1.0 / sqrt(2.0) : 1.0;
          sum += Cu * Cv * coeff[u][v] * cos(((2.0 * x + 1) * u * PI) / 16.0) *
                 cos(((2.0 * y + 1) * v * PI) / 16.0);
        }
      }
      matrix[i + x][j + y] = static_cast<float>(round(0.25 * sum));
    }
  }
}

vector<unsigned char> dct::performIDCT(vector<vector<float>> &yMatrix,
                                       vector<vector<float>> &cbMatrix,
                                       vector<vector<float>> &crMatrix) {
  for (int i = 0; i < yMatrix.size(); i += 8) {
    for (int j = 0; j < yMatrix[0].size(); j += 8) {
      idctOn8x8(i, j, yMatrix, "lume");
    }
  }
  for (int i = 0; i < cbMatrix.size(); i += 8) {
    for (int j = 0; j < cbMatrix[0].size(); j += 8) {
      idctOn8x8(i, j, cbMatrix, "chrome");
      idctOn8x8(i, j, crMatrix, "chrome");
    }
  }

  auto planeToBytes = [](vector<vector<float>> &plane) {
    vector<unsigned char> out;
    out.reserve(plane.size() * (plane.empty() ? 0 : plane[0].size()));
    for (auto &row : plane) {
      for (float val : row) {
        // undo the -128 level shift applied on encode, then clamp
        int pixel = static_cast<int>(round(val)) + 128;
        pixel = max(0, min(255, pixel));
        out.push_back(static_cast<unsigned char>(pixel));
      }
    }
    return out;
  };

  vector<unsigned char> yBytes = planeToBytes(yMatrix);
  vector<unsigned char> cbBytes = planeToBytes(cbMatrix);
  vector<unsigned char> crBytes = planeToBytes(crMatrix);

  vector<unsigned char> yuvBuffer;
  yuvBuffer.reserve(yBytes.size() + cbBytes.size() + crBytes.size());
  yuvBuffer.insert(yuvBuffer.end(), yBytes.begin(), yBytes.end());
  yuvBuffer.insert(yuvBuffer.end(), cbBytes.begin(), cbBytes.end());
  yuvBuffer.insert(yuvBuffer.end(), crBytes.begin(), crBytes.end());
  return yuvBuffer;
}
