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

  // Decode side: mirrors encode() in reverse. height/width are the
  // ORIGINAL (unpadded) dimensions -- same convention as encode(), since
  // the compressed file has no header carrying them (see DECODE_PLAN.md
  // "Open gaps"). Writes a raw RGB file to outputPath.
  void decode(unsigned int height, unsigned int width,
             string compressedFilePath, string outputPath);
};
