#pragma once

#include "chroma.hpp"
#include "dct.hpp"
#include "entropy.hpp"
class coder {
public:
  chroma chromaObj;
  dct dctObj;
  entropy entropyObj;

  vector<unsigned char> encode(unsigned int height, unsigned int width,
                               string file_path);
};
