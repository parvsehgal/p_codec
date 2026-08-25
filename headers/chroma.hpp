#pragma once
#include <string>
#include <vector>
using namespace std;
class chroma {
public:
  unsigned int height;
  unsigned int width;
  vector<unsigned char> readFile(unsigned int height, unsigned int width,
                                 std::string filePath);
  vector<unsigned char> rgbToyuv(std::vector<unsigned char> &rgbBuffer,
                                 unsigned int height, unsigned int width);
  vector<unsigned char> generateSubsample(unsigned int height,
                                          unsigned int width,
                                          std::string filePath);

  // --- Decode side ---

  // Nearest-neighbor upsamples a half-resolution Cb or Cr plane
  // (height/2 x width/2) back to full resolution, replicating each 4:2:0
  // sample across its source 2x2 region (inverse of the averaging done in
  // rgbToyuv).
  vector<unsigned char> upsampleChroma(vector<unsigned char> &chromaBuffer,
                                       unsigned int fullHeight,
                                       unsigned int fullWidth);

  // Inverts rgbToyuv's forward equations, per-pixel, on already
  // full-resolution Y/Cb/Cr buffers.
  vector<unsigned char> yuvToRgb(vector<unsigned char> &yBuffer,
                                 vector<unsigned char> &cbBuffer,
                                 vector<unsigned char> &crBuffer,
                                 unsigned int height, unsigned int width);

  // Crops a padded RGB buffer (paddedHeight x paddedWidth) back down to
  // origHeight x origWidth and writes it out as a raw RGB file.
  void writeFile(vector<unsigned char> &rgbBuffer, unsigned int paddedHeight,
                unsigned int paddedWidth, unsigned int origHeight,
                unsigned int origWidth, std::string outputPath);

  // Orchestrates the full decode-side chroma stage: yuv420p buffer
  // (already IDCT'd/dequantized) -> chroma upsample -> YCbCr->RGB -> strip
  // padding -> write raw file. Mirrors generateSubsample.
  void decode(vector<unsigned char> &yuvBuffer, unsigned int paddedHeight,
             unsigned int paddedWidth, unsigned int origHeight,
             unsigned int origWidth, std::string outputPath);
};
