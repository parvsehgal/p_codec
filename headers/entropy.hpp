#pragma once
#include <tuple>
#include <vector>

using namespace std;

class entropy {
public:
  void runLevelon8x8(int i, int j, vector<vector<float>> &Matrix,
                     vector<unsigned char> &encodedBitStream);

  void runLevel(const tuple<vector<vector<float>>, vector<vector<float>>,
                            vector<vector<float>>> &yuvmatrices);
};
