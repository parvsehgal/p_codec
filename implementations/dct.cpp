#include "../headers/dct.hpp"
#include <cstddef>
#include <iostream>
#include <vector>
#define PI 3.1415926535897

using namespace std;

void dct::dctOn8x8(unsigned int i, unsigned int j,
                   vector<vector<int>> &matrix) {
  float Cu, Cv;
  vector<vector<int>> input(8, vector<int>(8));
  for (int a = 0; a < 8; a++) {
    for (int b = 0; b < 8; b++) {
      input[a][b] = matrix[i + a][j + b];
    }
  }
  // now should be able to perform DCT and put the values directly in the source
  // matrix
  for (int u = 0; u < 8; u++) {
    for (int v = 0; v < 8; v++) {
      int sum = 0;
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
      matrix[u + i][v + j] = 1 / 4.0 * Cu * Cv * sum;
    }
  }
  if (i == 0 && j == 0) {
    cout << "below is input matrix" << endl;
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        cout << input[row][col] << " ";
      }
      cout << endl;
    }
    cout << "after DCT that same matrix" << endl;
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        cout << matrix[row + i][col + j] << " ";
      }
      cout << endl;
    }
  }
}
void dct::performDCT(vector<unsigned char> &imageSubSample, unsigned int width,
                     unsigned int height) {
  this->width = width;
  this->height = height;
  cout << "control is here----------------------------------" << endl;
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

  vector<vector<int>> yMatrix(height, vector<int>(width));
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      yMatrix[i][j] = static_cast<int>(yBuffer[i * width + j]);
    }
  }
  vector<vector<int>> cbMatrix(height / 2, vector<int>(width / 2));
  vector<vector<int>> crMatrix(height / 2, vector<int>(width / 2));
  for (int i = 0; i < (height / 2); i++) {
    for (int j = 0; j < (width / 2); j++) {
      cbMatrix[i][j] = static_cast<int>(cbBuffer[i * (width / 2) + j]);
      crMatrix[i][j] = static_cast<int>(crBuffer[i * (width / 2) + j]);
    }
  }

  cout << "yMatrix DIM" << yMatrix[0].size() << " " << yMatrix.size() << endl;
  cout << "cbMatrix DIM" << cbMatrix[0].size() << " " << cbMatrix.size()
       << endl;
  cout << "crMatrix DIM" << crMatrix[0].size() << " " << crMatrix.size()
       << endl;

  // all matrices made now perform DCT on 8x8 blocks

  for (int i = 0; i < yMatrix.size(); i += 8) {
    for (int j = 0; j < yMatrix[0].size(); j += 8) {
      dctOn8x8(i, j, yMatrix);
    }
  }
  for (int i = 0; i < cbMatrix.size(); i += 8) {
    for (int j = 0; j < cbMatrix[0].size(); j += 8) {
      dctOn8x8(i, j, cbMatrix);
      dctOn8x8(i, j, crMatrix);
    }
  }
  // now all matrices should have DCT applied on them
}
