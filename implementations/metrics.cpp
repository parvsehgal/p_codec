#include "../headers/metrics.hpp"
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

void metrics::calculateMetrics(std::string originalFile,
                               std::string decodedFile,
                               std::string encodedFile) {
  calculateCompressionRatio(originalFile, decodedFile, encodedFile);
  calculatePSNR(originalFile, decodedFile, encodedFile);
}
void metrics::calculateCompressionRatio(std::string originalFile,
                                        std::string decodedFile,
                                        std::string encodedFile) {
  // caculate compression ratio
  std::ifstream originalReader{originalFile, std::ios::binary | std::ios::ate};
  std::ifstream encodedReader{encodedFile, std::ios::binary | std::ios::ate};
  double compressionRatio =
      static_cast<double>(originalReader.tellg()) / encodedReader.tellg();
  std::cout << "the compression ratio is " << compressionRatio << "x"
            << std::endl;
}

void metrics::calculatePSNR(const std::string originalFile,
                            const std::string decodedFile,
                            const std::string encodedFile) {
  std::ifstream originalReader{originalFile, std::ios::binary};
  std::ifstream decodedReader{decodedFile, std::ios::binary};

  if (!originalReader || !decodedReader) {
    std::cerr << "Could not open one of the files\n";
    return;
  }

  // read both files fully into memory as bytes
  std::vector<unsigned char> original{
      std::istreambuf_iterator<char>(originalReader),
      std::istreambuf_iterator<char>()};
  std::vector<unsigned char> decoded{
      std::istreambuf_iterator<char>(decodedReader),
      std::istreambuf_iterator<char>()};

  if (original.empty() || original.size() != decoded.size()) {
    std::cerr << "Files are empty or have different sizes (" << original.size()
              << " vs " << decoded.size() << ")\n";
    return;
  }

  // mean squared error
  double sumSquaredError = 0.0;
  for (size_t i = 0; i < original.size(); ++i) {
    double diff = static_cast<double>(original[i]) - decoded[i];
    sumSquaredError += diff * diff;
  }
  double mse = sumSquaredError / original.size();

  if (mse == 0.0) {
    std::cout << "PSNR: infinite (images are identical, lossless)\n";
    return;
  }

  double psnr = 10.0 * std::log10((255.0 * 255.0) / mse);
  std::cout << "the PSNR is " << psnr << " dB\n";
}
