#pragma once
#include <string>
#include <tuple>
#include <vector>
using namespace std;
class dct {
public:
  unsigned int width;
  unsigned int height;
  vector<vector<int>> luminanceTable = {{16, 11, 10, 16, 24, 40, 51, 61},
                                        {12, 12, 14, 19, 26, 58, 60, 55},
                                        {14, 13, 16, 24, 40, 57, 69, 56},
                                        {14, 17, 22, 29, 51, 87, 80, 62},
                                        {18, 22, 37, 56, 68, 109, 103, 77},
                                        {24, 35, 55, 64, 81, 104, 113, 92},
                                        {49, 64, 78, 87, 103, 121, 120, 101},
                                        {72, 92, 95, 98, 112, 100, 103, 99}};
  vector<vector<int>> chrominanceTabe{
      {17, 18, 24, 47, 99, 99, 99, 99}, {18, 21, 26, 66, 99, 99, 99, 99},
      {24, 26, 56, 99, 99, 99, 99, 99}, {47, 66, 99, 99, 99, 99, 99, 99},
      {99, 99, 99, 99, 99, 99, 99, 99}, {99, 99, 99, 99, 99, 99, 99, 99},
      {99, 99, 99, 99, 99, 99, 99, 99}, {99, 99, 99, 99, 99, 99, 99, 99}};
  ;
  tuple<vector<vector<float>>, vector<vector<float>>, vector<vector<float>>>
  performDCT(vector<unsigned char> &imageSubSample, unsigned int width,
             unsigned int height);
  void dctOn8x8(unsigned int i, unsigned int j, vector<vector<float>> &maxtix,
                string component);

  // --- Decode side ---

  // Dequantizes (multiplies by the same quant table used on encode) and
  // performs the inverse DCT on the 8x8 block at matrix[i..i+7][j..j+7],
  // writing the spatial-domain residuals back in place (mirrors dctOn8x8).
  void idctOn8x8(unsigned int i, unsigned int j, vector<vector<float>> &matrix,
                 string component);

  // Runs idctOn8x8 over every 8x8 block of all three planes, undoes the
  // -128 level shift, clamps to 0..255, and concatenates Y, Cb, Cr into a
  // single yuv420p buffer (mirrors the output shape performDCT consumed).
  vector<unsigned char> performIDCT(vector<vector<float>> &yMatrix,
                                    vector<vector<float>> &cbMatrix,
                                    vector<vector<float>> &crMatrix);
};
