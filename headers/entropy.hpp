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
  bitReader bitReaderObj;
  vector<unsigned char> compressedFile;
  void huffmanEncode(int dc, vector<tuple<int, int, int>> &currRunLevel);
  vector<tuple<int, int, int>> runLevelon8x8(int i, int j,
                                             vector<vector<float>> &Matrix);
  vector<unsigned char>
  runLevel(const tuple<vector<vector<float>>, vector<vector<float>>,
                       vector<vector<float>>> &yuvmatrices,
           int width, int height);
  //==================DECODE FUNCTIONS===================================
  unordered_map<uint32_t, tuple<int, int, int>> reverseMap;
  void reverseTheMap();
  vector<pair<int, vector<tuple<int, int, int>>>>
  reverseEntropy(int width, int height, string compressedFileName);
  vector<pair<int, vector<tuple<int, int, int>>>>
  getAllPairs(int width, int height, vector<unsigned char> &fileBuffer);
  pair<int, vector<tuple<int, int, int>>> getBlock();
};
