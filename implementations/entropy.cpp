#include "../headers/entropy.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <tuple>
#include <vector>
using namespace std;

vector<vector<tuple<int, int, int>>>
entropy ::bytesToRunlevel(int width, int height,
                          vector<unsigned char> fileBuffer) {
  vector<vector<tuple<int, int, int>>> res;
  // remove the dimensions and start to read the file
  int dimensionsQty = 4;
  while (dimensionsQty--)
    fileBuffer.erase(fileBuffer.begin());
  cout << "file without dims " << fileBuffer.size() << endl;
  // make a bit reader
  // this->bitReaderObj.reader(fileBuffer);

  return res;
}
void entropy::huffmanEncode(int dc,
                            vector<tuple<int, int, int>> &currRunLevel) {
  // add the DC like straight up as an unsigned char
  // then findd the vlc value for the other pairs | take into account the escape
  // code
  char DC = dc;
  this->bitWriterObj.addBits(DC, 8);
  bool hasAC = !(currRunLevel.size() == 1 && get<2>(currRunLevel[0]) == 0 &&
                 get<1>(currRunLevel[0]) == 0);
  this->bitWriterObj.addBits(hasAC ? 1 : 0, 1);
  if (hasAC) {
    // start encoding
    for (int i = 0; i < currRunLevel.size(); i++) {
      auto [last, run, level] = currRunLevel[i];
      if (this->tcoeffTable.vlc_table.find({abs(last), abs(run), abs(level)}) !=
          tcoeffTable.vlc_table.end()) {
        // a match is found
        // add bits
        uint32_t bitsToAppend =
            this->tcoeffTable.vlc_table[{abs(last), abs(run), abs(level)}]
                .first;
        if (level < 0) {
          bitsToAppend = bitsToAppend << 1 | 1;
        } else {
          bitsToAppend = bitsToAppend << 1 | 0;
        }
        int len = this->tcoeffTable.vlc_table[{abs(last), abs(run), abs(level)}]
                      .second;
        this->bitWriterObj.addBits(bitsToAppend, len + 1);
      } else {
        // use escape code logic
        auto [escBits, escLen] = this->tcoeffTable.escapeCode;
        this->bitWriterObj.addBits(escBits, escLen);                // marker
        this->bitWriterObj.addBits(static_cast<uint32_t>(last), 1); // LAST
        this->bitWriterObj.addBits(static_cast<uint32_t>(run), 6);  // RUN
        this->bitWriterObj.addBits(static_cast<uint32_t>(level) & 0xFF, 8);
      }
    }
  }
}

vector<tuple<int, int, int>>
entropy::runLevelon8x8(int first, int second, vector<vector<float>> &Matrix) {
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
  // this is for testing
  if (first == 0 && second == 0) {
    for (int i = 0; i < zigzag.size(); i++) {
      cout << zigzag[i] << " ";
    }
    cout << endl;
  }
  //-------------------------
  vector<tuple<int, int, int>> runLevelPairs;
  int zeroCount = 0;
  // we are skipping the DC and handeling that seprately
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
  int currDc;
  if (!zigzag.empty()) {
    currDc = zigzag[0];
  }
  if (first == 0 && second == 0) {
    cout << "curr DC " << currDc << endl;
  }
  // now return the zigzag array for huffman encoding
  return runLevelPairs;
}
// slit the tuple
// then do 8x8 zig zag run level and huffman coding and append into a
// combined bitstream
string entropy::dimensionsToBinaryLiteral(uint32_t num) {
  string res = "";
  for (int i = 31; i >= 0; i--)
    res += ((num >> i) & 1) ? '1' : '0';
  return res;
}
vector<unsigned char>
entropy::runLevel(const tuple<vector<vector<float>>, vector<vector<float>>,
                              vector<vector<float>>> &yuvMatrices,
                  int width, int height) {
  cout << "control in entropy class" << endl;
  // first add the dimensions of the raw image as a sort of pseudo header to the
  // bitstream
  this->bitWriterObj.addBits(width, 16);
  this->bitWriterObj.addBits(height, 16);
  // no need to flush since this is clean 4 bits

  // go over the matrices in 8x8 chunks to read them in zig zig order
  auto yMatrix = get<0>(yuvMatrices);
  auto cbMatrix = get<1>(yuvMatrices);
  auto crMatrix = get<2>(yuvMatrices);
  for (int i = 0; i < yMatrix.size(); i += 8) {
    for (int j = 0; j < yMatrix[0].size(); j += 8) {
      vector<tuple<int, int, int>> currRunLevel = runLevelon8x8(i, j, yMatrix);
      int dc = yMatrix[i][j];
      huffmanEncode(dc, currRunLevel);
    }
  }
  for (int i = 0; i < cbMatrix.size(); i += 8) {
    for (int j = 0; j < cbMatrix[0].size(); j += 8) {
      int dc = cbMatrix[i][j];
      vector<tuple<int, int, int>> currRunLevel = runLevelon8x8(i, j, cbMatrix);
      huffmanEncode(dc, currRunLevel);
    }
  }
  for (int i = 0; i < crMatrix.size(); i += 8) {
    for (int j = 0; j < crMatrix[0].size(); j += 8) {
      int dc = crMatrix[i][j];
      vector<tuple<int, int, int>> currRunLevel = runLevelon8x8(i, j, crMatrix);
      huffmanEncode(dc, currRunLevel);
    }
  }
  this->bitWriterObj.flush();
  vector<unsigned char> compressedFile = std::move(this->bitWriterObj.buffer);
  this->bitWriterObj.buffer.clear();
  return compressedFile;
}

void entropy::reverseEntropy(int width, int height, string compressedFileName) {
  // make run level last pairs from bytes of the the file
  ifstream compressedFile{compressedFileName, std::ios::binary | std::ios::ate};
  size_t fileSize = compressedFile.tellg();
  compressedFile.seekg(0, std::ios::beg);
  cout << fileSize << endl;
  vector<unsigned char> fileBuffer(fileSize);
  compressedFile.read(reinterpret_cast<char *>(fileBuffer.data()), fileSize);
  vector<vector<tuple<int, int, int>>> allPairs =
      bytesToRunlevel(width, height, fileBuffer);
}
