#include "../headers/entropy.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <tuple>
#include <unordered_map>
#include <vector>
using namespace std;

// this functions encodes an 8x8 block
void entropy::huffmanEncode(int dc,
                            vector<tuple<int, int, int>> &currRunLevel) {
  // add the DC like straight up as an unsigned char
  // then findd the vlc value for the other pairs | take into account the
  // escape code
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
vector<unsigned char>
entropy::runLevel(const tuple<vector<vector<float>>, vector<vector<float>>,
                              vector<vector<float>>> &yuvMatrices,
                  int width, int height) {
  cout << "control in entropy class" << endl;
  // first add the dimensions of the raw image as a sort of pseudo header to
  // the bitstream
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
//=======================================DECODE FUNCTIONS=====================
pair<int, vector<tuple<int, int, int>>> entropy::getBlock() {
  // get DC
  pair<int, vector<tuple<int, int, int>>> res;
  int DC = 0;
  int k = 8;
  while (k--) {
    DC = (DC << 1) | this->bitReaderObj.getBit();
  }
  if (DC & 0x80)
    DC -= 256;
  bool hasAC = this->bitReaderObj.getBit();
  uint32_t currBits = 0;
  int len = 0;
  int kMaxLen = 12;
  res.first = DC;
  if (!hasAC) {
    return {DC, {{1, 0, 0}}};
  }
  while (len < kMaxLen) {
    currBits = (currBits << 1) | this->bitReaderObj.getBit();
    len++;
    uint32_t toCheck = (currBits << 5) | static_cast<uint32_t>(len);
    auto it = this->reverseMap.find(toCheck);
    if (it != this->reverseMap.end()) {
      cout << "NORMAL CODE ENCOUNTERED +++++++++" << endl;
      auto toPush = it->second;
      // get the signBit as well
      int signBit = this->bitReaderObj.getBit();
      if (signBit) {
        res.second.push_back({get<0>(toPush), get<1>(toPush), -get<2>(toPush)});
      } else {
        res.second.push_back({get<0>(toPush), get<1>(toPush), get<2>(toPush)});
      }
      if (get<0>(toPush) == 1) {
        return res; // last == 1, end of block
      }
      currBits = 0;
      len = 0;
    } else if (len == this->tcoeffTable.escapeCode.second &&
               currBits == this->tcoeffTable.escapeCode.first) {
      // this is an escape code
      cout << "ESCAPE CODE ENCOUNTERED ---------" << endl;
      int last = this->bitReaderObj.getBit();
      int run = 0;
      int level = 0;
      for (int i = 0; i < 6; i++) {
        run = (run << 1) | this->bitReaderObj.getBit();
      }
      for (int i = 0; i < 8; i++) {
        level = (level << 1) | this->bitReaderObj.getBit();
      }
      if (level & 0x80)
        level -= 256;
      res.second.push_back({last, run, level});
      currBits = 0;
      len = 0;
      if (last == 1)
        return res; // end of block
    }
  }
  throw std::runtime_error(
      "entropy::getBlock: no matching VLC or escape code found within " +
      std::to_string(kMaxLen) + " bits (bitstream desync or bad table)");
}

void entropy::reverseTheMap() {
  for (const auto &entry : this->tcoeffTable.vlc_table) {
    auto [last, run, level] = entry.first;
    auto [bits, len] = entry.second;
    uint32_t key = (bits << 5) | static_cast<uint32_t>(len);
    this->reverseMap[key] = {last, run, level};
  }
}
int blocksPerPlane(int W, int H) {
  int blocksWide = (W + 7) / 8;
  int blocksHigh = (H + 7) / 8;
  return blocksWide * blocksHigh;
}
vector<pair<int, vector<tuple<int, int, int>>>>
entropy::getAllPairs(int width, int height, vector<unsigned char> &fileBuffer) {
  // first make a reverse vlc_table
  reverseTheMap();
  vector<pair<int, vector<tuple<int, int, int>>>> res;
  // remove the dimesntions/header from the buffer
  int k = 4;
  while (k--) {
    fileBuffer.erase(fileBuffer.begin());
  }
  this->bitReaderObj.init(fileBuffer);
  // get the total number of blocks
  int totalBlocks =
      blocksPerPlane(width, height)                            // Y
      + 2 * blocksPerPlane((width + 1) / 2, (height + 1) / 2); // CB + CR
  cout << "total Block count ? " << totalBlocks << endl;
  // start building res block by block
  for (int i = 0; i < totalBlocks; i++) {
    auto currBlock = getBlock();
    res.push_back(currBlock);
  }
  return res;
}

void entropy::reverseEntropy(int width, int height, string compressedFileName) {
  // make run level last pairs from bytes of the the file
  ifstream compressedFile{compressedFileName, std::ios::binary | std::ios::ate};
  size_t fileSize = compressedFile.tellg();
  compressedFile.seekg(0, std::ios::beg);
  vector<unsigned char> fileBuffer(fileSize);
  compressedFile.read(reinterpret_cast<char *>(fileBuffer.data()), fileSize);

  vector<pair<int, vector<tuple<int, int, int>>>> allPairs =
      getAllPairs(width, height, fileBuffer);
  cout << "big if here" << endl;
}
