#pragma once
#include "bitWriter.hpp"
#include "tcoeffTableClass.hpp"
#include <tuple>
#include <vector>

using namespace std;

class entropy {
public:
  tcoeffTableClass tcoeffTable;
  bitWriter bitWriterObj;
  vector<unsigned char> compressedFile;

  void huffmanEncode(int dc, vector<tuple<int, int, int>> &currRunLevel);

  vector<tuple<int, int, int>> runLevelon8x8(int i, int j,
                                             vector<vector<float>> &Matrix);

  vector<unsigned char>
  runLevel(const tuple<vector<vector<float>>, vector<vector<float>>,
                       vector<vector<float>>> &yuvmatrices);
};
