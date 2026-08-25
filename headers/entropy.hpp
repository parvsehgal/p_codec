#pragma once
#include "bitReader.hpp"
#include "bitWriter.hpp"
#include "tcoeffTableClass.hpp"
#include <tuple>
#include <unordered_map>
#include <vector>
using namespace std;
class entropy {
public:
  tcoeffTableClass tcoeffTable;
  bitWriter bitWriterObj;
  vector<unsigned char> compressedFile;

  // (bits << 5 | length) -> (last, run, level). Max VLC length in the table
  // is 12 bits, so 5 bits of room for length is plenty.
  unordered_map<uint32_t, tuple<int, int, int>> reverseVlcTable;
  bool reverseVlcTableBuilt = false;
  void buildReverseVlcTable();

  void huffmanEncode(int dc, vector<tuple<int, int, int>> &currRunLevel);
  vector<tuple<int, int, int>> runLevelon8x8(int i, int j,
                                             vector<vector<float>> &Matrix);
  vector<unsigned char>
  runLevel(const tuple<vector<vector<float>>, vector<vector<float>>,
                       vector<vector<float>>> &yuvmatrices);

  // --- Decode side ---

  // Returns the exact (row, col) scan order runLevelon8x8 used, so decode
  // can scatter the rebuilt zigzag array back into an 8x8 block the same
  // way it was gathered.
  static vector<pair<int, int>> zigzagOrder();

  // Decodes one 8x8 block's worth of coefficients from the bitstream and
  // writes them (still quantized, zigzag-order rebuilt into row/col
  // position) into matrix[i..i+7][j..j+7].
  void decodeBlock(bitReader &reader, vector<vector<float>> &matrix, int i,
                    int j);

  tuple<vector<vector<float>>, vector<vector<float>>, vector<vector<float>>>
  runLevelDecode(const vector<unsigned char> &compressedFile,
                 unsigned int width, unsigned int height);
};
