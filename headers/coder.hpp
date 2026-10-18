#pragma once

#include "chroma.hpp"
#include "dct.hpp"
#include "entropy.hpp"
#include "metrics.hpp"
class coder {
public:
  chroma chromaObj;
  dct dctObj;
  entropy entropyObj;
  metrics metricsObj;

  vector<unsigned char> encode(unsigned int height, unsigned int width,
                               string file_path);

  void decode(string compressedFileName);

  void calculateMetrics(string originalFile, string decodedFile,
                        string encodedFile);
};
