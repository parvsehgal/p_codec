#include <string>
class metrics {
public:
  void calculateMetrics(std::string originalFile, std::string decodedFile,
                        std::string encodedFile);
  void calculatePSNR(std::string originalFile, std::string decodedFile,
                     std::string encodedFile);
  void calculateCompressionRatio(std::string originalFile,
                                 std::string decodedFile,
                                 std::string encodedFile);
};
