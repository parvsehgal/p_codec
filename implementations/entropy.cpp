#include "../headers/entropy.hpp"
#include <cstdint>
#include <iostream>
#include <tuple>
#include <vector>
using namespace std;

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
        this->bitWriterObj.addBits(escBits, escLen);
        // marker
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
  if (first == 0 && second == 0) {
    for (int i = 0; i < zigzag.size(); i++) {
      cout << zigzag[i] << " ";
    }
    cout << endl;
  }
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
                              vector<vector<float>>> &yuvMatrices) {
  cout << "control in entropy class" << endl;
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

// ============================= DECODE SIDE =============================

void entropy::buildReverseVlcTable() {
  if (reverseVlcTableBuilt)
    return;
  for (const auto &entry : tcoeffTable.vlc_table) {
    auto [last, run, level] = entry.first;
    auto [bits, len] = entry.second;
    uint32_t key = (bits << 5) | static_cast<uint32_t>(len);
    reverseVlcTable[key] = {last, run, level};
  }
  reverseVlcTableBuilt = true;
}

// Replays the exact same diagonal zigzag traversal runLevelon8x8 used, but
// records (row, col) positions instead of reading matrix values, so decode
// can scatter the rebuilt 64-element array back to the right cells.
vector<pair<int, int>> entropy::zigzagOrder() {
  vector<pair<int, int>> order;
  int n = 8;
  int m = 8;
  int row = 0, col = 0;
  bool row_inc = 0;
  int mn = min(m, n);
  for (int len = 1; len <= mn; ++len) {
    for (int i = 0; i < len; ++i) {
      order.push_back({row, col});
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
      order.push_back({row, col});
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
  return order;
}

void entropy::decodeBlock(bitReader &reader, vector<vector<float>> &matrix,
                          int i, int j) {
  buildReverseVlcTable();
  static const vector<pair<int, int>> order = zigzagOrder();

  // DC: 8 bits, signed (mirrors `char DC = dc` on encode).
  int8_t dcRaw = static_cast<int8_t>(reader.readBits(8));
  int dc = dcRaw;

  vector<int> zigzag(64, 0);
  zigzag[0] = dc;

  int hasAC = reader.readBit();
  if (hasAC) {
    auto [escBits, escLen] = tcoeffTable.escapeCode;
    int idx = 1;
    bool last = false;
    // Keep decoding (last,run,level) pairs until we hit one with last==1,
    // matching how huffmanEncode wrote them.
    while (!last) {
      uint32_t code = 0;
      int len = 0;
      bool matched = false;
      int curLast = 0, run = 0, level = 0;
      while (len < 16) {
        int bit = reader.readBit();
        code = (code << 1) | static_cast<uint32_t>(bit);
        len++;
        if (code == static_cast<uint32_t>(escBits) && len == escLen) {
          curLast = reader.readBit();
          run = static_cast<int>(reader.readBits(6));
          int8_t lvl = static_cast<int8_t>(reader.readBits(8));
          level = lvl;
          matched = true;
          break;
        }
        uint32_t key = (code << 5) | static_cast<uint32_t>(len);
        auto it = reverseVlcTable.find(key);
        if (it != reverseVlcTable.end()) {
          auto [l, r, lv] = it->second;
          int sign = reader.readBit();
          curLast = l;
          run = r;
          level = sign ? -lv : lv;
          matched = true;
          break;
        }
      }
      if (!matched) {
        // Malformed / truncated stream -- bail out of this block rather
        // than spinning forever.
        break;
      }
      for (int z = 0; z < run && idx < 64; z++) {
        zigzag[idx++] = 0;
      }
      if (idx < 64) {
        zigzag[idx++] = level;
      }
      last = (curLast == 1);
      if (idx >= 64) {
        break;
      }
    }
  }

  for (int k = 0; k < 64; k++) {
    auto [r, c] = order[k];
    matrix[i + r][j + c] = static_cast<float>(zigzag[k]);
  }
}

tuple<vector<vector<float>>, vector<vector<float>>, vector<vector<float>>>
entropy::runLevelDecode(const vector<unsigned char> &compressedFile,
                        unsigned int width, unsigned int height) {
  bitReader reader(compressedFile);

  vector<vector<float>> yMatrix(height, vector<float>(width, 0));
  vector<vector<float>> cbMatrix(height / 2, vector<float>(width / 2, 0));
  vector<vector<float>> crMatrix(height / 2, vector<float>(width / 2, 0));

  // Same order the encoder used: all Y blocks, then all Cb, then all Cr.
  for (int i = 0; i < yMatrix.size(); i += 8) {
    for (int j = 0; j < yMatrix[0].size(); j += 8) {
      decodeBlock(reader, yMatrix, i, j);
    }
  }
  for (int i = 0; i < cbMatrix.size(); i += 8) {
    for (int j = 0; j < cbMatrix[0].size(); j += 8) {
      decodeBlock(reader, cbMatrix, i, j);
    }
  }
  for (int i = 0; i < crMatrix.size(); i += 8) {
    for (int j = 0; j < crMatrix[0].size(); j += 8) {
      decodeBlock(reader, crMatrix, i, j);
    }
  }

  return {yMatrix, cbMatrix, crMatrix};
}
