#include "../headers/dct.hpp"
#include <cmath>
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
  return {yMatrix, cbMatrix, crMatrix};
}
//==================DECODE FUNCTIONS===================================
void dct::Compute8x8Idct(vector<vector<float>> &in, vector<vector<float>> &out,
                         int lumeChrome) {
  int i, j, u, v;
  float s;
  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++) {
      s = 0;
      for (u = 0; u < 8; u++)
        for (v = 0; v < 8; v++) {
          int quant;
          if (!lumeChrome) {
            quant = this->luminanceTable[u][v];
          } else {
            quant = this->chrominanceTabe[u][v];
          }
          s += quant * in[u][v] * cos((2 * i + 1) * u * PI / 16) *
               cos((2 * j + 1) * v * PI / 16) * ((u == 0) ? 1 / sqrt(2) : 1.) *
               ((v == 0) ? 1 / sqrt(2) : 1.);
        }
      out[i][j] = (s / 4);
    }
}
void dct::zigzagFill(vector<float> &arr, vector<vector<float>> &mat) {
  int row = 0, col = 0;
  bool up = true; // true = moving up-right, false = moving down-left
  for (int i = 0; i < 64; ++i) {
    mat[row][col] = arr[i];
    if (up) {
      if (col == 7) {
        ++row;
        up = false;
      } // hit right edge
      else if (row == 0) {
        ++col;
        up = false;
      } // hit top edge
      else {
        --row;
        ++col;
      }
    } else {
      if (row == 7) {
        ++col;
        up = true;
      } // hit bottom edge
      else if (col == 0) {
        ++row;
        up = true;
      } // hit left edge
      else {
        ++row;
        --col;
      }
    }
  }
}
vector<vector<float>> dct::getMatrixFromBlock(block &currBlock) {
  vector<vector<float>> res(8, vector<float>(8, 0)); // 8x8 empty matrix
  res[0][0] = currBlock.first;
  vector<float> zigzigArr;
  zigzigArr.push_back(currBlock.first);
  for (auto pair : currBlock.second) {
    int run = get<1>(pair);
    while (run--) {
      zigzigArr.push_back(0);
    }
    zigzigArr.push_back(get<2>(pair));
    if (get<0>(pair) == 1) {
      while (zigzigArr.size() < 64) {
        zigzigArr.push_back(0);
      }
      break;
    }
  }
  zigzagFill(zigzigArr, res);
  return res;
}

void dct::IDCTon8x8(int i, int j, vector<vector<float>> &Matrix, int lumeChrome,
                    block currBlock) {
  vector<vector<float>> preIdctMatrix = getMatrixFromBlock(currBlock);
  vector<vector<float>> postIdctMatrix(preIdctMatrix);
  Compute8x8Idct(preIdctMatrix, postIdctMatrix, lumeChrome);
  for (int a = 0; a < 8; a++) {
    for (int b = 0; b < 8; b++) {
      Matrix[a + i][b + j] = postIdctMatrix[a][b];
    }
  }
}
float dct::clampPixel(float shiftedPixel) {
  if (shiftedPixel < 0.0)
    shiftedPixel = 0.0f;
  if (shiftedPixel > 255.0)
    shiftedPixel = 255.0f;
  return round(shiftedPixel);
}
vector<unsigned char> dct::performIDCT(int width, int height,
                                       blocks &decodedBlocks) {
  vector<vector<float>> yMatrix(height, vector<float>(width));
  vector<vector<float>> cbMatrix(height / 2, vector<float>(width / 2));
  vector<vector<float>> crMatrix(height / 2, vector<float>(width / 2));

  int blockCounter = 0;
  for (int i = 0; i < yMatrix.size(); i += 8) {
    for (int j = 0; j < yMatrix[0].size(); j += 8) {
      IDCTon8x8(i, j, yMatrix, 0, decodedBlocks[blockCounter++]);
    }
  }
  for (int i = 0; i < cbMatrix.size(); i += 8) {
    for (int j = 0; j < cbMatrix[0].size(); j += 8) {
      // single 8x8 matrix from block and IDCT + reverse Quantization
      IDCTon8x8(i, j, cbMatrix, 1, decodedBlocks[blockCounter++]);
    }
  }
  for (int i = 0; i < crMatrix.size(); i += 8) {
    for (int j = 0; j < crMatrix[0].size(); j += 8) {
      // single 8x8 matrix from block and IDCT + reverse Quantization
      IDCTon8x8(i, j, crMatrix, 1, decodedBlocks[blockCounter++]);
    }
  }
  vector<unsigned char> decodedSubSample;
  for (int i = 0; i < yMatrix.size(); i++) {
    for (int j = 0; j < yMatrix[0].size(); j++) {
      decodedSubSample.push_back(
          static_cast<unsigned char>(clampPixel(yMatrix[i][j] + 128)));
    }
  }
  for (int i = 0; i < cbMatrix.size(); i++) {
    for (int j = 0; j < cbMatrix[0].size(); j++) {
      decodedSubSample.push_back(
          static_cast<unsigned char>(clampPixel(cbMatrix[i][j] + 128)));
    }
  }
  for (int i = 0; i < crMatrix.size(); i++) {
    for (int j = 0; j < crMatrix[0].size(); j++) {
      decodedSubSample.push_back(
          static_cast<unsigned char>(clampPixel(crMatrix[i][j] + 128)));
    }
  }
  return decodedSubSample;
}
