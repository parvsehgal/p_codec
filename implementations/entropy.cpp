#include "../headers/entropy.hpp"
#include <iostream>
#include <tuple>
#include <vector>
using namespace std;

void entropy::runLevelon8x8(int first, int second,
                            vector<vector<float>> &Matrix,
                            vector<unsigned char> &encodedBitStream) {
  // zigzag Scan of 8x8 matrix
  vector<int> zigzag;
  int n = 8;
  int m = 8;
  int row = 0, col = 0;
  bool row_inc = 0;
  int mn = min(m, n);
  for (int len = 1; len <= mn; ++len) {
    for (int i = 0; i < len; ++i) {
      zigzag.push_back(Matrix[row + first][col + second]);

      if (i + 1 == len)
        break;
      if (row_inc)
        ++row, --col;
      else
        --row, ++col;
    }

    if (len == mn)
      break;

    if (row_inc)
      ++row, row_inc = false;
    else
      ++col, row_inc = true;
  }
  if (row == 0) {
    if (col == m - 1)
      ++row;
    else
      ++col;
    row_inc = 1;
  } else {
    if (row == n - 1)
      ++col;
    else
      ++row;
    row_inc = 0;
  }
  int MAX = max(m, n) - 1;
  for (int len, diag = MAX; diag > 0; --diag) {
    len = (diag > mn) ? mn : diag;
    for (int i = 0; i < len; ++i) {
      zigzag.push_back(Matrix[row + first][col + second]);

      if (i + 1 == len)
        break;
      if (row_inc)
        ++row, --col;
      else
        ++col, --row;
    }
    if (row == 0 || col == m - 1) {
      if (col == m - 1)
        ++row;
      else
        ++col;
      row_inc = true;
    } else if (col == 0 || row == n - 1) {
      if (row == n - 1)
        ++col;
      else
        ++row;
      row_inc = false;
    }
  }
  if (first == 0 && second == 0) {
    for (int i = 0; i < zigzag.size(); i++) {
      cout << zigzag[i] << " ";
    }
    cout << endl;
  }
  // convert the zigzag to run level pairs
  // i starts as 1 so as to avoid DC and only pick up AC
  vector<tuple<int, int, int>> runLevelPairs;
  int zeroCount = 0;
  for (int i = 1; i < zigzag.size(); i++) {
    if (zigzag[i] == 0) {
      zeroCount++;
    } else {
      runLevelPairs.push_back({0, zeroCount, zigzag[i]});
      zeroCount = 0;
    }
  }
  if (!runLevelPairs.empty()) {
    get<0>(runLevelPairs.back()) = 1;
  } else {
    runLevelPairs.push_back({1, 0, 0});
  }
  if (first == 0 && second == 0) {
    for (int i = 0; i < runLevelPairs.size(); i++) {
      auto [last, run, level] = runLevelPairs[i];
      cout << last << " " << run << " " << level << endl;
    }
  }
  // now huffman encode and append to encodedBitStream
  //
}
// slit the tuple
// then do 8x8 zig zag run level and huffman coding and append into a
// combined bitstream
void entropy::runLevel(const tuple<vector<vector<float>>, vector<vector<float>>,
                                   vector<vector<float>>> &yuvMatrices) {

  vector<unsigned char> encodedBitStream;
  cout << "control in entropy class" << endl;
  // go over the matrices in 8x8 chunks to read them in zig zig order
  auto yMatrix = get<0>(yuvMatrices);
  auto cbMatrix = get<1>(yuvMatrices);
  auto crMatrix = get<2>(yuvMatrices);
  for (int i = 0; i < yMatrix.size(); i += 8) {
    for (int j = 0; j < yMatrix[0].size(); j += 8) {
      runLevelon8x8(i, j, yMatrix, encodedBitStream);
    }
  }
  for (int i = 0; i < cbMatrix.size(); i += 8) {
    for (int j = 0; j < cbMatrix[0].size(); j += 8) {
      runLevelon8x8(i, j, cbMatrix, encodedBitStream);
    }
  }
  for (int i = 0; i < crMatrix.size(); i += 8) {
    for (int j = 0; j < crMatrix[0].size(); j += 8) {
      runLevelon8x8(i, j, crMatrix, encodedBitStream);
    }
  }
}
