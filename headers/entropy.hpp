#pragma once
#include "bitReader.hpp"
#include "bitWriter.hpp"
#include "tcoeffTableClass.hpp"
#include <tuple>
#include <vector>

using namespace std;

class entropy {
public:
  tcoeffTableClass tcoeffTable;
  bitWriter bitWriterObj;

  bitReader bitReaderObj;

  vector<unsigned char> compressedFile;

  string dimensionsToBinaryLiteral(uint32_t num);
  void huffmanEncode(int dc, vector<tuple<int, int, int>> &currRunLevel);

  vector<tuple<int, int, int>> runLevelon8x8(int i, int j,
                                             vector<vector<float>> &Matrix);

  vector<unsigned char>
  runLevel(const tuple<vector<vector<float>>, vector<vector<float>>,
                       vector<vector<float>>> &yuvmatrices,
           int width, int height);
  void reverseEntropy(int width, int height, string compressedFileName);

  vector<vector<tuple<int, int, int>>>
  bytesToRunlevel(int width, int height, vector<unsigned char> fileBuffer);
};
